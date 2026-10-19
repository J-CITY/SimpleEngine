#include "contentService.h"
#include "fileSystem/storage.h"
#include "fileSystem/userStorage.h"
#include "resourceResolver.h"
#include <regex>
#include <set>

namespace IKIGAI::RESOURCES {
ContentService::ContentService(FileSystem& files, std::filesystem::path root, std::string platform)
	: mFiles(files)
	, mRoot(std::move(root))
	, mPlatform(std::move(platform)) { }

nlohmann::json ContentService::validate(const nlohmann::json& manifest) const {
	if (manifest.at("schemaVersion") != 1 || manifest.at("minEngineVersion").get<int>() > 1)
		throw std::runtime_error("Unsupported content schema/engine version");
	const auto platform = manifest.at("platform").get<std::string>();
	if (platform != "any" && platform != mPlatform)
		throw std::runtime_error("Content platform mismatch");
	if (!std::regex_match(manifest.at("releaseId").get<std::string>(), std::regex("[a-zA-Z0-9_-]{1,80}")))
		throw std::runtime_error("Invalid release ID");
	if (!manifest.at("archives").is_array() || manifest.at("archives").empty() || manifest.at("archives").size() > 128
		|| !manifest.at("files").is_object())
		throw std::runtime_error("Invalid release catalog");
	for (const auto& archive : manifest.at("archives")) {
		const auto hash = archive.at("sha256").get<std::string>();
		if (!std::regex_match(hash, std::regex("[0-9a-f]{64}")))
			throw std::runtime_error("Invalid archive identity");
		if (archive.at("path") != "archives/" + hash + ".zip" || archive.at("size").get<uint64_t>() > (uint64_t { 1 } << 34))
			throw std::runtime_error("Invalid archive path/size");
		for (const auto& item : archive.value("deletedPaths", nlohmann::json::array()))
			FileSystem::normalizePath(item.get<std::string>());
	}
	for (const auto& [key, value] : manifest.at("files").items()) {
		const auto path = FileSystem::normalizePath(key);
		if (path == "/" || path.starts_with("/user/") || path.starts_with("/cache/")
			|| !std::regex_match(value.get<std::string>(), std::regex("[0-9a-f]{64}")))
			throw std::runtime_error("Invalid file catalog entry");
	}
	return manifest;
}

void ContentService::verify(const nlohmann::json& manifest) const {
	validate(manifest);
	FileSystem candidate;
	for (const auto& archive : manifest.at("archives")) {
		const auto path = STORAGE::Child(mRoot, archive.at("path").get<std::string>());
		if (std::filesystem::file_size(path) != archive.at("size").get<uint64_t>() || STORAGE::Sha256(path) != archive.at("sha256"))
			throw std::runtime_error("Archive integrity mismatch");
		candidate.mountZip(path.generic_string(), "/", archive.value("deletedPaths", std::vector<std::string> { }));
	}
	// Read every visible file to verify ZIP CRC/decompression and the catalog.
	std::set<std::string> visible;
	std::function<void(std::string)> walk = [&](const auto& dir) {
		for (const auto& entry : candidate.listDirectory(dir)) {
			if (entry.isDirectory)
				walk(entry.path);
			else
				visible.insert(entry.path.substr(1));
		}
	};
	walk("/");
	if (visible.size() != manifest.at("files").size())
		throw std::runtime_error("Release file count mismatch");
	for (const auto& name : visible) {
		if (!manifest.at("files").contains(name))
			throw std::runtime_error("Unlisted release file");
		auto file = candidate.getFile(name);
		const auto bytes = file->read();
		if (bytes.size() != file->getSize())
			throw std::runtime_error("Corrupt archive entry");
		if (STORAGE::Sha256(std::span<const uint8_t>(bytes)) != manifest.at("files").at(name))
			throw std::runtime_error("Release file hash mismatch: " + name);
	}
	if (!candidate.isFileExist("/resource-map.json"))
		throw std::runtime_error("Missing resource map");
	const auto mapping = nlohmann::json::parse(candidate.getFile("/resource-map.json")->readStr());
	ResourceResolver resolver(candidate);
	for (const auto& [key, value] : mapping.at("textures").items())
		resolver.texture(key);
}

void ContentService::stageLocal(const std::filesystem::path& path) {
	const auto manifest = validate(nlohmann::json::parse(STORAGE::Read(path)));
	for (const auto& archive : manifest.at("archives")) {
		const auto relative = archive.at("path").get<std::string>();
		const auto source = STORAGE::Child(path.parent_path(), relative);
		const auto target = STORAGE::Child(mRoot, relative);
		if (std::filesystem::file_size(source) != archive.at("size").get<uint64_t>() || STORAGE::Sha256(source) != archive.at("sha256"))
			throw std::runtime_error("Source archive integrity mismatch");
		std::filesystem::create_directories(target.parent_path());
		if (std::filesystem::is_regular_file(target) && std::filesystem::file_size(target) == archive.at("size").get<uint64_t>()
			&& STORAGE::Sha256(target) == archive.at("sha256"))
			continue;
		if (std::filesystem::weakly_canonical(source) != std::filesystem::weakly_canonical(target)) {
			auto temporary = target;
			temporary += ".install";
			std::filesystem::copy_file(source, temporary, std::filesystem::copy_options::overwrite_existing);
			if (std::filesystem::exists(target))
				std::filesystem::remove(target);
			std::filesystem::rename(temporary, target);
		}
	}
	verify(manifest);
	STORAGE::WriteTextAtomic(mRoot / "releases" / (manifest.at("releaseId").get<std::string>() + ".json"), manifest.dump(2));
}

std::vector<NETWORK::DownloadHandle> ContentService::download(
	NETWORK::DownloadManager& manager, const nlohmann::json& trustedManifest, const std::string& baseUrl, bool allowLocalhost) {
	const auto manifest = validate(trustedManifest);
	std::vector<NETWORK::DownloadHandle> result;
	for (const auto& archive : manifest.at("archives")) {
		const auto relative = archive.at("path").get<std::string>();
		const auto hash = archive.at("sha256").get<std::string>();
		NETWORK::DownloadRequest request;
		request.destination = STORAGE::Child(mRoot, relative);
		request.size = archive.at("size").get<uint64_t>();
		request.identity = hash;
		request.http.url = baseUrl + (baseUrl.ends_with('/') ? "" : "/") + relative;
		request.http.allowInsecureLocalhost = allowLocalhost;
		request.verify = [hash](const auto& path) { return STORAGE::Sha256(path) == hash; };
		request.commit = [] { return STORAGE::CommitPersistent(); };
		result.push_back(manager.download(std::move(request)));
	}
	return result;
}

std::shared_future<void> ContentService::activate(const nlohmann::json& manifest) {
	verify(manifest);
	std::vector<FileSystem::MountId> next;
	try {
		for (const auto& archive : manifest.at("archives"))
			next.push_back(mFiles.mountZip(STORAGE::Child(mRoot, archive.at("path").get<std::string>()).generic_string(), "/",
				archive.value("deletedPaths", std::vector<std::string> { })));
		const auto active = mRoot / "active.json";
		if (std::filesystem::is_regular_file(active)) {
			try {
				const auto bytes = STORAGE::Read(active);
				const auto old = nlohmann::json::parse(bytes);
				verify(old);
				if (old != manifest)
					STORAGE::WriteAtomic(mRoot / "previous.json", bytes);
			} catch (const std::exception&) { /* Preserve the known-good previous snapshot during recovery. */
			}
		}
		STORAGE::WriteTextAtomic(active, manifest.dump(2));
	} catch (...) {
		for (auto id : next)
			mFiles.unmount(id);
		throw;
	}
	for (auto id : mMounts)
		mFiles.unmount(id);
	mMounts = std::move(next);
	mRelease = manifest.at("releaseId").get<std::string>();
	return STORAGE::CommitPersistent();
}

bool ContentService::restore() {
	for (const auto& name : { "active.json", "previous.json" }) {
		const auto path = mRoot / name;
		if (!std::filesystem::is_regular_file(path))
			continue;
		try {
			const auto manifest = nlohmann::json::parse(STORAGE::Read(path));
			activate(manifest);
			return true;
		} catch (const std::exception&) { }
	}
	if (std::filesystem::exists(mRoot / "active.json") || std::filesystem::exists(mRoot / "previous.json"))
		throw std::runtime_error("No valid installed content release");
	return false;
}
}
