#include "fileSystem.h"
#include "nativeFile.h"
#include "storage.h"
#include "userStorage.h"
#include "vfsCompatibility.h"
#include "vfspp/VFS.h"
#ifdef USE_SDL
#include "sdlFileSystem.h"
#endif
#ifdef OCULUS
#include "androidAssetFileSystem.h"
#include <android/asset_manager.h>
#endif
#include <algorithm>
#include <atomic>
#include <cctype>
#include <mutex>
#include <set>
#include <stdexcept>

namespace IKIGAI::RESOURCES {
namespace {
	bool Contains(const std::string& base, const std::string& path) {
		return base == "/" || path == base || path.starts_with(base + "/");
	}
	bool Reserved(const std::string& path) {
		return Contains("/user", path) || Contains("/cache", path);
	}
	struct Mount {
		FileSystem::MountInfo info;
		vfspp::IFileSystemPtr fs;
		std::shared_ptr<STORAGE::UserStorage> storage;
		std::string prefix;
		std::string storagePath(const std::string& path) const {
			const auto relative = STORAGE::Utf8(std::filesystem::u8path(path).lexically_relative(std::filesystem::u8path(info.alias)));
			return relative == "." ? prefix : prefix + "/" + relative;
		}
	};
	std::optional<vfspp::EntryInfo> Entry(const Mount& mount, const std::string& path) {
		if (path == mount.info.alias)
			return vfspp::EntryInfo(mount.info.alias, mount.info.source, path, vfspp::EntryType::Directory);
		if (mount.storage) {
			const auto key = mount.storagePath(path);
			if (!mount.storage->exists(key))
				return { };
			return vfspp::EntryInfo(
				mount.info.alias, "", path, mount.storage->isDirectory(key) ? vfspp::EntryType::Directory : vfspp::EntryType::File);
		}
		if (std::dynamic_pointer_cast<vfspp::NativeFileSystem>(mount.fs)) {
			vfspp::EntryInfo entry(mount.info.alias, mount.info.source, path);
			std::error_code ec;
			const auto relative = std::filesystem::path(path).lexically_relative(mount.info.alias).generic_string();
			const auto status = std::filesystem::status(STORAGE::Child(std::filesystem::u8path(mount.info.source), relative), ec);
			if (ec || (!std::filesystem::is_regular_file(status) && !std::filesystem::is_directory(status)))
				return { };
			return vfspp::EntryInfo(mount.info.alias, mount.info.source, path,
				std::filesystem::is_directory(status) ? vfspp::EntryType::Directory : vfspp::EntryType::File);
		}
		return mount.fs->GetEntryInfo(path);
	}
}
class FileSystemInternal {
public:
	std::vector<std::shared_ptr<Mount>> mounts;
	mutable std::recursive_mutex mutex;
	uint64_t nextId = 1;
	std::atomic<uint64_t> generation { 0 };
	FileSystem::MountId add(vfspp::IFileSystemPtr fs, std::vector<std::string> deleted = { }) {
		if (!fs->Initialize())
			throw std::runtime_error("Cannot initialize filesystem: " + fs->BasePath());
		std::lock_guard lock(mutex);
		auto alias = FileSystem::normalizePath(fs->VirtualPath());
		auto record = std::make_shared<Mount>(Mount { { nextId++, fs->BasePath(), alias, fs->IsReadOnly(), std::move(deleted) }, fs });
		mounts.push_back(record);
		++generation;
		return record->info.id;
	}
	std::vector<std::shared_ptr<Mount>> ordered(const std::string& path) const {
		std::lock_guard lock(mutex);
		std::vector<std::shared_ptr<Mount>> result;
		for (auto it = mounts.rbegin(); it != mounts.rend(); ++it)
			if (Contains((*it)->info.alias, path))
				result.push_back(*it);
		std::stable_sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a->info.alias.size() > b->info.alias.size(); });
		return result;
	}
	std::shared_ptr<Mount> resolve(const std::string& path) const {
		for (const auto& mount : ordered(path)) {
			if (Reserved(path) && mount->info.alias == "/")
				return { };
			if (Entry(*mount, path))
				return mount;
			for (const auto& deleted : mount->info.deletedPaths)
				if (Contains(deleted, path))
					return { };
			auto parent = std::filesystem::path(path).parent_path();
			while (!parent.empty() && Contains(mount->info.alias, parent.generic_string())) {
				const auto entry = Entry(*mount, parent.generic_string());
				if (entry && !entry->IsDirectory())
					return { };
				if (parent == parent.root_path())
					break;
				parent = parent.parent_path();
			}
			if (Reserved(path))
				return { };
		}
		return { };
	}
};
class FileInternal {
public:
	// vfspp ZipFile holds a weak archive reference. Retain the owning mount.
	std::shared_ptr<Mount> owner;
	vfspp::IFilePtr file;
	bool closed = false;
	FileInternal(vfspp::IFilePtr value, std::shared_ptr<Mount> mount)
		: owner(std::move(mount))
		, file(std::move(value)) { }
};
std::string FileSystem::normalizePath(std::string path) {
	std::replace(path.begin(), path.end(), '\\', '/');
	if (path.find(':') != std::string::npos || path.find('\0') != std::string::npos || path.starts_with("//"))
		throw std::runtime_error("Expected a virtual path: " + path);
	for (const auto& part : std::filesystem::u8path(path))
		if (part == "..")
			throw std::runtime_error("Parent traversal in virtual path: " + path);
	if (path.empty() || path.front() != '/')
		path.insert(path.begin(), '/');
	path = STORAGE::Utf8(std::filesystem::u8path(path).lexically_normal());
	while (path.size() > 1 && path.back() == '/')
		path.pop_back();
	return path;
}
File::File(std::unique_ptr<FileInternal> internal)
	: mInternal(std::move(internal)) { }
File::~File() = default;
bool File::isValid() const {
	return isOpened();
}
bool File::isOpened() const {
	return !mInternal->closed && mInternal->file && mInternal->file->IsOpened();
}
bool File::isReadOnly() const {
	return !mInternal->file || mInternal->owner->info.readOnly || bool(mInternal->owner->storage);
}
bool File::isDir() const {
	return mInternal->file && mInternal->file->GetEntryInfo().IsDirectory();
}
size_t File::getSize() const {
	return mInternal->file ? mInternal->file->Size() : 0;
}
size_t File::tell() const {
	return mInternal->file ? mInternal->file->Tell() : 0;
}
void File::seek(size_t offset) {
	if (!isOpened() || mInternal->file->Seek(offset, vfspp::IFile::Origin::Begin) != offset)
		throw std::runtime_error("Cannot seek file");
}
std::string File::getFileExtension() const {
	return mInternal->file ? mInternal->file->GetEntryInfo().Extension() : "";
}
std::string File::getFileName() const {
	return mInternal->file ? mInternal->file->GetEntryInfo().Filename() : "";
}
std::string File::getAbsolutePath() const {
	return mInternal->file ? mInternal->file->GetEntryInfo().NativePath() : "";
}
void File::open(FileMode mode) const {
	if (!mInternal->file || (isReadOnly() && (static_cast<uint8_t>(mode) & 2)) || !mInternal->file->Open(static_cast<vfspp::IFile::FileMode>(mode)))
		throw std::runtime_error("Cannot open file");
	mInternal->closed = false;
}
void File::close() const {
	if (mInternal->file)
		mInternal->file->Close();
	mInternal->closed = true;
}
std::vector<uint8_t> File::read(size_t size) {
	std::vector<uint8_t> result;
	if (isOpened())
		result.resize(static_cast<size_t>(mInternal->file->Read(result, size)));
	return result;
}
std::vector<uint8_t> File::read() {
	return read(getSize() - std::min(getSize(), tell()));
}
std::vector<uint8_t> File::readChunk(size_t size) {
	if (!isOpened())
		throw std::runtime_error("File is not open");
	return read(size);
}
std::string File::readStr() {
	const auto bytes = read();
	return { bytes.begin(), bytes.end() };
}
void File::read(uint8_t* data, size_t size) {
	if (!isOpened() || mInternal->file->Read(std::span<uint8_t>(data, size)) != size)
		throw std::runtime_error("Short file read");
}
void File::write(const uint8_t* data, size_t size) {
	if (!isOpened() || isReadOnly() || mInternal->file->Write(std::span<const uint8_t>(data, size)) != size)
		throw std::runtime_error("Short file write");
}
COROUTINE::Task<std::vector<uint8_t>> File::readAllTask(std::shared_ptr<ResourceLoadControl> control) {
	const auto size = getSize() - std::min(getSize(), tell());
	std::vector<uint8_t> bytes;
	bytes.reserve(size);
	while (bytes.size() < size) {
		if (control)
			control->check();
		auto chunk = readChunk(std::min<size_t>(64 * 1024, size - bytes.size()));
		if (chunk.empty())
			throw std::runtime_error("Unexpected EOF: " + getFileName());
		bytes.insert(bytes.end(), chunk.begin(), chunk.end());
		co_await COROUTINE::NextFrame();
	}
	co_return bytes;
}
FileSystem::FileSystem()
	: mInternal(std::make_unique<FileSystemInternal>()) { }
FileSystem::~FileSystem() = default;
FileSystem::MountId FileSystem::mountNative(const std::string& path, const std::string& alias, bool readOnly) {
	return mInternal->add(std::make_shared<vfspp::NativeFileSystem>(normalizePath(alias), path, readOnly));
}
FileSystem::MountId FileSystem::mountStorage(std::shared_ptr<STORAGE::UserStorage> storage, const std::string& prefix, const std::string& alias) {
	STORAGE::Child(storage->root(), prefix);
	if (!storage->isDirectory(prefix))
		throw std::runtime_error("Storage mount directory is missing");
	std::lock_guard lock(mInternal->mutex);
	const auto id = mInternal->add(std::make_shared<vfspp::MemoryFileSystem>(normalizePath(alias)));
	auto& mount = mInternal->mounts.back();
	mount->storage = std::move(storage);
	mount->prefix = prefix;
	mount->info.source = "storage:" + prefix;
	return id;
}
FileSystem::MountId FileSystem::mountZip(const std::string& path, const std::string& alias, const std::vector<std::string>& deletedPaths) {
	const auto normalized = normalizePath(alias);
	if (Reserved(normalized))
		throw std::runtime_error("Archive cannot mount over user storage");
	std::vector<std::string> deleted;
	for (const auto& item : deletedPaths) {
		auto key = normalizePath(item);
		if (key == "/" || Reserved(key) || !Contains(normalized, key))
			throw std::runtime_error("Invalid deletion: " + item);
		deleted.push_back(key);
	}
	mz_zip_archive zip { };
	if (!mz_zip_reader_init_file(&zip, path.c_str(), 0))
		throw std::runtime_error("Invalid ZIP: " + path);
	struct Close {
		mz_zip_archive* zip;
		~Close() {
			mz_zip_reader_end(zip);
		}
	} close { &zip };
	std::set<std::string> names;
	uint64_t total = 0;
	const auto count = mz_zip_reader_get_num_files(&zip);
	if (count > 100000)
		throw std::runtime_error("Too many ZIP entries");
	for (mz_uint i = 0; i < count; ++i) {
		mz_zip_archive_file_stat stat { };
		if (!mz_zip_reader_file_stat(&zip, i, &stat))
			throw std::runtime_error("Invalid ZIP entry");
		const std::string raw(stat.m_filename);
		if (raw.empty() || raw.front() == '/' || raw.find('\\') != std::string::npos)
			throw std::runtime_error("Invalid ZIP path: " + raw);
		auto key = normalizePath((normalized == "/" ? "" : normalized) + "/" + raw);
		if (Reserved(key))
			throw std::runtime_error("ZIP contains reserved path");
		std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (!names.insert(key).second)
			throw std::runtime_error("Duplicate ZIP path: " + raw);
		if (stat.m_uncomp_size > (uint64_t { 1 } << 32) || total > (uint64_t { 1 } << 35) - stat.m_uncomp_size)
			throw std::runtime_error("ZIP size limit exceeded");
		total += stat.m_uncomp_size;
	}
	return mInternal->add(std::make_shared<vfspp::ZipFileSystem>(normalized, path), std::move(deleted));
}
void FileSystem::addNativeFileSystem(const std::string& path, const std::string& alias) {
	mountNative(path, alias);
}
void FileSystem::addZipFileSystem(const std::string& path, const std::string& alias) {
	mountZip(path, alias);
}
void FileSystem::addMemoryFileSystem(const std::string& alias) {
	mInternal->add(std::make_shared<vfspp::MemoryFileSystem>(normalizePath(alias)));
}
void FileSystem::addSdlFileSystem(const std::string& path, const std::string& alias, bool readOnly) {
#ifdef USE_SDL
	mInternal->add(std::make_shared<SdlFileSystem>(normalizePath(alias), path, readOnly));
#else
	throw std::runtime_error("SDL filesystem unavailable");
#endif
}
void FileSystem::addAndroidAssetFileSystem(const std::string& path, const std::string& alias, void* assets, void* app) {
#ifdef OCULUS
	mInternal->add(std::make_shared<AndroidAssetFileSystem>(normalizePath(alias), path, static_cast<AAssetManager*>(assets), app));
#else
	throw std::runtime_error("Android filesystem unavailable");
#endif
}
bool FileSystem::unmount(MountId id) {
	std::lock_guard lock(mInternal->mutex);
	const auto size = mInternal->mounts.size();
	std::erase_if(mInternal->mounts, [id](const auto& mount) { return mount->info.id == id; });
	if (mInternal->mounts.size() == size)
		return false;
	++mInternal->generation;
	return true;
}
std::vector<FileSystem::MountInfo> FileSystem::mounts() const {
	std::lock_guard lock(mInternal->mutex);
	std::vector<MountInfo> result;
	for (const auto& mount : mInternal->mounts)
		result.push_back(mount->info);
	return result;
}
uint64_t FileSystem::generation() const {
	return mInternal->generation;
}
bool FileSystem::isValid(const std::string& path) const {
	return bool(mInternal->resolve(normalizePath(path)));
}
bool FileSystem::isFileExist(const std::string& path) const {
	const auto key = normalizePath(path);
	const auto mount = mInternal->resolve(key);
	return mount && !Entry(*mount, key)->IsDirectory();
}
bool FileSystem::isDir(const std::string& path) const {
	const auto key = normalizePath(path);
	const auto mount = mInternal->resolve(key);
	return mount && Entry(*mount, key)->IsDirectory();
}
std::string FileSystem::getFileExtension(const std::string& path) const {
	return STORAGE::Utf8(std::filesystem::u8path(normalizePath(path)).extension());
}
std::string FileSystem::getFileName(const std::string& path) const {
	return STORAGE::Utf8(std::filesystem::u8path(normalizePath(path)).filename());
}
std::optional<std::string> FileSystem::getAbsolutePath(const std::string& path) const {
	const auto key = normalizePath(path);
	const auto mount = mInternal->resolve(key);
	if (mount && std::dynamic_pointer_cast<vfspp::NativeFileSystem>(mount->fs))
		return Entry(*mount, key)->NativePath();
	return { };
}
std::optional<std::string> FileSystem::getFilePath(const std::string& path) const {
	const auto key = normalizePath(path);
	return isValid(key) ? std::optional(key) : std::nullopt;
}
FileSystem::FileTime FileSystem::lastWriteTime(const std::string& path) {
	const auto native = getAbsolutePath(path);
	std::error_code ec;
	return native ? std::filesystem::last_write_time(std::filesystem::u8path(*native), ec) : FileTime { };
}
uintmax_t FileSystem::fileSize(const std::string& path) {
	if (const auto native = getAbsolutePath(path)) {
		std::error_code error;
		const auto size = std::filesystem::file_size(std::filesystem::u8path(*native), error);
		if (!error)
			return size;
	}
	return getFile(path)->getSize();
}
std::shared_ptr<File> FileSystem::getFile(const std::string& path, FileMode mode) {
	const auto key = normalizePath(path);
	auto mount = mInternal->resolve(key);
	const bool writing = (static_cast<uint8_t>(mode) & 2) != 0;
	if (!mount && writing) {
		const auto candidates = mInternal->ordered(key);
		if (!candidates.empty())
			mount = candidates.front();
	}
	vfspp::IFilePtr file;
	if (mount && !isDir(key) && (!Reserved(key) || mount->info.alias != "/") && (!writing || !mount->info.readOnly)) {
		if (mount->storage) {
			// Mutable streaming handles have no portable async commit contract.
			// Use writeAtomic/profile.writeFile for writes to this mount.
			if (!writing && mount->storage->exists(mount->storagePath(key))) {
				auto object = std::make_shared<vfspp::MemoryFileObject>();
				*object->GetWritableData() = mount->storage->read(mount->storagePath(key));
				auto handle = std::make_shared<vfspp::MemoryFile>(vfspp::EntryInfo(mount->info.alias, "", key), object);
				if (handle->Open(vfspp::IFile::FileMode::Read))
					file = handle;
			}
		}
		else if (std::dynamic_pointer_cast<vfspp::NativeFileSystem>(mount->fs)) {
			const auto relative = std::filesystem::path(key).lexically_relative(mount->info.alias).generic_string();
			STORAGE::Child(std::filesystem::u8path(mount->info.source), relative);
			auto handle = std::make_shared<NativeDiskFile>(vfspp::EntryInfo(mount->info.alias, mount->info.source, key));
			if (handle->Open(static_cast<vfspp::IFile::FileMode>(mode)))
				file = handle;
		}
		else
			file = mount->fs->OpenFile(key, static_cast<vfspp::IFile::FileMode>(mode));
	}
	return std::make_shared<File>(std::make_unique<FileInternal>(std::move(file), std::move(mount)));
}
std::shared_ptr<File> FileSystem::getLiveFile(const std::string& path) {
	auto file = getFile(path);
	return file->isValid() ? file : nullptr;
}
std::vector<FileSystem::DirectoryEntry> FileSystem::listDirectory(const std::string& path) const {
	const auto key = normalizePath(path);
	if (!isDir(key))
		return { };
	std::set<std::string> candidates;
	for (const auto& mount : mInternal->ordered(key)) {
		if (mount->storage) {
			if (!mount->storage->isDirectory(mount->storagePath(key)))
				continue;
			for (const auto& entry : mount->storage->list(mount->storagePath(key)))
				candidates.insert(normalizePath(key + "/" + entry.name));
		}
		else if (std::dynamic_pointer_cast<vfspp::NativeFileSystem>(mount->fs)) {
			const vfspp::EntryInfo entry(mount->info.alias, mount->info.source, key);
			std::error_code ec;
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator it(std::filesystem::u8path(entry.NativePath()), ec); !ec && it != end; it.increment(ec))
				candidates.insert(normalizePath((key == "/" ? "" : key) + "/" + it->path().filename().generic_string()));
		}
		else
			for (const auto& entry : mount->fs->GetEntriesList(false)) {
				const auto candidate = normalizePath(entry.VirtualPath());
				if (std::filesystem::path(candidate).parent_path().generic_string() == key)
					candidates.insert(candidate);
			}
	}
	std::vector<DirectoryEntry> result;
	for (const auto& candidate : candidates)
		if (auto mount = mInternal->resolve(candidate))
			result.push_back({ candidate, getFileName(candidate), Entry(*mount, candidate)->IsDirectory() });
	return result;
}
std::vector<FileSystem::DirectoryEntry> FileSystem::listLiveDirectory(const std::string& path) const {
	return listDirectory(path);
}
std::shared_future<void> FileSystem::writeAtomic(const std::string& path, std::span<const uint8_t> data) {
	const auto key = normalizePath(path);
	const auto candidates = mInternal->ordered(key);
	if (isDir(key))
		throw std::runtime_error("Cannot replace a VFS directory with a file");
	auto mount = mInternal->resolve(key);
	if (!mount && !candidates.empty())
		mount = candidates.front();
	if (mount && mount->storage) {
		mount->storage->writeAtomic(mount->storagePath(key), data);
		return mount->storage->commit();
	}
	if (!mount || (Reserved(key) && mount->info.alias == "/") || mount->info.readOnly
		|| !std::dynamic_pointer_cast<vfspp::NativeFileSystem>(mount->fs))
		throw std::runtime_error("No writable native mount: " + path);
	const vfspp::EntryInfo entry(mount->info.alias, mount->info.source, key);
	STORAGE::WriteAtomic(std::filesystem::u8path(entry.NativePath()), data);
	return STORAGE::ReadyCommit();
}
std::shared_future<void> FileSystem::writeTextAtomic(const std::string& path, const std::string& text) {
	const auto value = text + "\n";
	return writeAtomic(path, { reinterpret_cast<const uint8_t*>(value.data()), value.size() });
}
}
