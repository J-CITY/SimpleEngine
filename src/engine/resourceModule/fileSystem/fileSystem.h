#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <future>
#include "resourceModule/resourceRequest.h"

namespace IKIGAI::RESOURCES {
	namespace STORAGE { class UserStorage; }
	class FileSystemInternal;
	class FileInternal;

	enum class FileMode : uint8_t {
		READ = 1,
		WRITE = 2,
		READ_WRITE = READ | WRITE,
		APPEND = 4,
		TRUNCATE = 8
	};

	class File {
		std::unique_ptr<FileInternal> mInternal;

		std::vector<uint8_t> read(size_t sz);
		void read(uint8_t* data, size_t sz);
		void write(const uint8_t* data, size_t sz);
	public:
		File(std::unique_ptr<FileInternal> internal);
        ~File();
        size_t tell() const;
        void seek(size_t offset);
		bool isValid() const;
		std::string getFileExtension() const;
		std::string getFileName() const;
		std::string getAbsolutePath() const;
		bool isDir() const;
		bool isReadOnly() const;
		size_t getSize() const;
		bool isOpened() const;
		void open(FileMode mode) const;
		void close() const;

		std::vector<uint8_t> read();
        std::vector<uint8_t> readChunk(size_t maxBytes);
        COROUTINE::Task<std::vector<uint8_t>> readAllTask(std::shared_ptr<ResourceLoadControl> control = {});
		std::string readStr();
		template<typename T>
		void read(T& value) {
			read(reinterpret_cast<uint8_t*>(&value), sizeof(value));
		}

		template<typename T>
		void write(const T& value) {
			write(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
		}
		void write(const std::string& value) {
			write(reinterpret_cast<const uint8_t*>(value.data()), value.size());
		}
		void write(const std::vector<uint8_t>& value) {
			write(value.data(), value.size());
		}
	};

	class FileSystem {
		std::unique_ptr<FileSystemInternal> mInternal;
	public:
		using FileTime = std::filesystem::file_time_type;
        using MountId = uint64_t;
        struct MountInfo {
            MountId id;
            std::string source, alias;
            bool readOnly;
            std::vector<std::string> deletedPaths;
        };

		FileSystem();
		~FileSystem();

		void addNativeFileSystem(const std::string& path, const std::string& pathInFs);
        MountId mountNative(const std::string& path, const std::string& alias = "/", bool readOnly = false);
        MountId mountStorage(std::shared_ptr<STORAGE::UserStorage> storage, const std::string& prefix, const std::string& alias);
        MountId mountZip(const std::string& path, const std::string& alias = "/", const std::vector<std::string>& deletedPaths = {});
        bool unmount(MountId id);
        std::vector<MountInfo> mounts() const;
        uint64_t generation() const;
        static std::string normalizePath(std::string path);
		void addZipFileSystem(const std::string& path, const std::string& pathInFs);
		void addMemoryFileSystem(const std::string& pathInFs);
        void addSdlFileSystem(const std::string& path, const std::string& pathInFs, bool readOnly = false);
		void addAndroidAssetFileSystem(const std::string& path, const std::string& pathInFs, void* assetManager, void* androidApp);

		bool isValid(const std::string& path) const;
		bool isFileExist(const std::string& path) const;
		std::string getFileExtension(const std::string& path) const;
		std::string getFileName(const std::string& path) const;
		std::optional<std::string> getAbsolutePath(const std::string& path) const;
		bool isDir(const std::string& path) const;

		FileTime lastWriteTime(const std::string& path);
		uintmax_t fileSize(const std::string& path);

		struct DirectoryEntry {
			std::string path;
			std::string name;
			bool isDirectory = false;
		};

		std::shared_ptr<File> getFile(const std::string& path, FileMode mode = FileMode::READ);
		std::optional<std::string> getFilePath(const std::string& path) const;
		std::vector<DirectoryEntry> listDirectory(const std::string& path) const;
        // Tooling reads that discover files created/deleted after native mount
        // initialization, while retaining VFS mount precedence and packaged IO.
        std::shared_ptr<File> getLiveFile(const std::string& path);
        std::vector<DirectoryEntry> listLiveDirectory(const std::string& path) const;
        // Storage mounts include persistent commit in the returned completion.
        std::shared_future<void> writeTextAtomic(const std::string& path, const std::string& text);
        std::shared_future<void> writeAtomic(const std::string& path, std::span<const uint8_t> data);
	};
}
