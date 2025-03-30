#include "fileSystem.h"
#include "vfspp/VFS.h"

namespace IKIGAI::RESOURCES {
	class FileSystemInternal {
	public:
		vfspp::VirtualFileSystemPtr mVFS;
		FileSystemInternal() : mVFS(new vfspp::VirtualFileSystem()) {}
	};

	class FileInternal {
	public:
		vfspp::IFilePtr  mFile;
		FileInternal(vfspp::IFilePtr file) : mFile(file) {}
	};
}

IKIGAI::RESOURCES::File::File(std::unique_ptr<FileInternal> internal): mInternal(std::move(internal)) {
}

bool IKIGAI::RESOURCES::File::isValid() const {
	return mInternal->mFile->GetFileInfo().IsValid();
}

std::string IKIGAI::RESOURCES::File::getFileExtension() const {
	return mInternal->mFile->GetFileInfo().Extension();
}

std::string IKIGAI::RESOURCES::File::getFileName() const {
	return mInternal->mFile->GetFileInfo().Name();
}

std::string IKIGAI::RESOURCES::File::getAbsolutePath() const {
	return mInternal->mFile->GetFileInfo().AbsolutePath();
}

bool IKIGAI::RESOURCES::File::isDir() const {
	return mInternal->mFile->GetFileInfo().IsDir();
}

bool IKIGAI::RESOURCES::File::isReadOnly() const {
	return mInternal->mFile->IsReadOnly();
}

size_t IKIGAI::RESOURCES::File::getSize() const {
	return mInternal->mFile->Size();
}

bool IKIGAI::RESOURCES::File::isOpened() const {
	return mInternal->mFile->IsOpened();
}

std::vector<uint8_t> IKIGAI::RESOURCES::File::read(size_t sz) {
	std::vector<uint8_t> data;
	mInternal->mFile->Read(data, sz);
	return data;
}

std::vector<uint8_t> IKIGAI::RESOURCES::File::read() {
	return read(getSize());
}

std::string IKIGAI::RESOURCES::File::readStr() {
	auto data = read();
	return std::string(data.begin(), data.end());
}

void IKIGAI::RESOURCES::File::read(uint8_t* data, size_t sz) {
	mInternal->mFile->Read(data, sz);
}

void IKIGAI::RESOURCES::File::write(const uint8_t* data, size_t sz) {
	mInternal->mFile->Write(data, sz);
}


IKIGAI::RESOURCES::FileSystem::FileSystem(): mInternal(std::make_unique<FileSystemInternal>()) {

}

void IKIGAI::RESOURCES::FileSystem::addNativeFileSystem(const std::string& path, const std::string& pathInFs) {
	vfspp::IFileSystemPtr fs(new vfspp::NativeFileSystem(path));
	fs->Initialize();
	mInternal->mVFS->AddFileSystem(pathInFs, fs);
}

void IKIGAI::RESOURCES::FileSystem::addZipFileSystem(const std::string& path, const std::string& pathInFs) {
	vfspp::IFileSystemPtr fs(new vfspp::ZipFileSystem(path));
	fs->Initialize();
	mInternal->mVFS->AddFileSystem(pathInFs, fs);
}

void IKIGAI::RESOURCES::FileSystem::addMemoryFileSystem(const std::string& pathInFs) {
	vfspp::IFileSystemPtr fs(new vfspp::MemoryFileSystem());
	fs->Initialize();
	mInternal->mVFS->AddFileSystem(pathInFs, fs);
}

bool IKIGAI::RESOURCES::FileSystem::isValid(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.IsValid();
}

bool IKIGAI::RESOURCES::FileSystem::isFileExist(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.IsFileExist();
}

std::string IKIGAI::RESOURCES::FileSystem::getFileExtension(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.Extension();
}

std::string IKIGAI::RESOURCES::FileSystem::getFileName(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.Name();
}

std::string IKIGAI::RESOURCES::FileSystem::getAbsolutePath(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.AbsolutePath();
}

bool IKIGAI::RESOURCES::FileSystem::isDir(const std::string& path) const {
	auto info = vfspp::FileInfo(path);
	return info.IsDir();
}

std::shared_ptr<IKIGAI::RESOURCES::File> IKIGAI::RESOURCES::FileSystem::getFile(const std::string& path, FileMode mode) {
	vfspp::IFilePtr file = mInternal->mVFS->OpenFile(vfspp::FileInfo(path), static_cast<vfspp::IFile::FileMode>(mode));
	return std::make_shared<File>(std::make_unique<FileInternal>(file));
}
