#include "resourceResolver.h"
namespace IKIGAI::RESOURCES {
ResolvedTexture ResourceResolver::texture(const std::string& logicalPath) {
	if (mGeneration != mFiles.generation()) {
		auto entries = nlohmann::json::object();
		if (auto file = mFiles.getFile("/resource-map.json"); file->isValid()) {
			const auto data = nlohmann::json::parse(file->readStr());
			if (data.at("schemaVersion") != 1 || !data.at("textures").is_object())
				throw std::runtime_error("Unsupported resource map");
			for (const auto& [name, value] : data.at("textures").items())
				entries[FileSystem::normalizePath(name)] = value;
		}
		mTextures = std::move(entries);
		mGeneration = mFiles.generation();
	}
	const auto key = FileSystem::normalizePath(logicalPath);
	if (!mTextures.contains(key))
		return { key };
	const auto& item = mTextures.at(key);
	const auto kind = item.at("kind").get<std::string>();
	auto path = FileSystem::normalizePath(item.at("path").get<std::string>());
	if (!mFiles.isFileExist(path))
		throw std::runtime_error("Unresolved texture mapping: " + key);
	if (kind == "file")
		return { path };
	if (kind != "atlas")
		throw std::runtime_error("Unknown texture mapping kind");
	auto metadata = FileSystem::normalizePath(item.at("metadata").get<std::string>());
	auto file = mFiles.getFile(metadata);
	if (!file->isValid())
		throw std::runtime_error("Missing atlas metadata");
	const auto atlas = nlohmann::json::parse(file->readStr());
	auto region = item.at("region").get<std::string>();
	if (!atlas.at("Files").contains(region))
		throw std::runtime_error("Missing atlas region: " + region);
	if (std::filesystem::path(path).replace_extension(".atlas").generic_string() != metadata)
		throw std::runtime_error("Atlas metadata must accompany its image");
	return { path, metadata, region, true };
}
}
