#include "sdlFileSystem.h"
#include <algorithm>
#include <vector>
#include <filesystem>

namespace IKIGAI::RESOURCES {

// --- SdlFile ---

SdlFile::SdlFile(const vfspp::FileInfo& fileInfo)
    : m_fileInfo(fileInfo)
{}

SdlFile::~SdlFile() {
    SdlFile::Close();
}

const vfspp::FileInfo& SdlFile::GetFileInfo() const {
    return m_fileInfo;
}

uint64_t SdlFile::Size() {
    if (m_rwops) {
        return static_cast<uint64_t>(SDL_RWsize(m_rwops));
    }
    return 0;
}

bool SdlFile::IsReadOnly() const {
    std::error_code ec;
    auto perms = std::filesystem::status(m_fileInfo.AbsolutePath(), ec).permissions();
    if (ec) {
        // Если путь не поддерживается (например, Android apk assets),
        // файл гарантированно только для чтения.
        return true; 
    }
    return (perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none;
}

std::string SdlFile::getSdlMode(FileMode mode) const {
    std::string strMode = "rb";
    if ((mode & FileMode::ReadWrite) == FileMode::ReadWrite) {
        strMode = "r+b";
    } else if ((mode & FileMode::Write) == FileMode::Write) {
        strMode = "wb";
    }

    if ((mode & FileMode::Append) == FileMode::Append) {
        strMode = "ab";
    }
    return strMode;
}

void SdlFile::Open(FileMode mode) {
    if (IsOpened()) {
        Close();
    }
    m_rwops = SDL_RWFromFile(m_fileInfo.AbsolutePath().c_str(), getSdlMode(mode).c_str());
}

void SdlFile::Close() {
    if (m_rwops) {
        SDL_RWclose(m_rwops);
        m_rwops = nullptr;
    }
}

bool SdlFile::IsOpened() const {
    return m_rwops != nullptr;
}

uint64_t SdlFile::Seek(uint64_t offset, Origin origin) {
    if (!m_rwops) return 0;
    
    int whence = RW_SEEK_SET;
    if (origin == Origin::Begin) {
        whence = RW_SEEK_SET;
    } else if (origin == Origin::Set) {
        whence = RW_SEEK_CUR;
    } else if (origin == Origin::End) {
        whence = RW_SEEK_END;
    }

    SDL_RWseek(m_rwops, static_cast<Sint64>(offset), whence);
    return Tell();
}

uint64_t SdlFile::Tell() {
    if (!m_rwops) return 0;
    return static_cast<uint64_t>(SDL_RWtell(m_rwops));
}

uint64_t SdlFile::Read(uint8_t* buffer, uint64_t size) {
    if (!m_rwops) return 0;
    return static_cast<uint64_t>(SDL_RWread(m_rwops, buffer, 1, static_cast<size_t>(size)));
}

uint64_t SdlFile::Write(const uint8_t* buffer, uint64_t size) {
    if (!m_rwops) return 0;
    return static_cast<uint64_t>(SDL_RWwrite(m_rwops, buffer, 1, static_cast<size_t>(size)));
}

uint64_t SdlFile::Read(std::vector<uint8_t>& buffer, uint64_t size) {
    buffer.resize(size);
    return Read(buffer.data(), size);
}

uint64_t SdlFile::Write(const std::vector<uint8_t>& buffer) {
    return Write(buffer.data(), buffer.size());
}

uint64_t SdlFile::Read(std::ostream& stream, uint64_t size, uint64_t bufferSize) {
    uint64_t totalSize = size;
    std::vector<uint8_t> buffer(bufferSize);
    while (size > 0) {
        uint64_t bytesRead = Read(buffer.data(), std::min(size, static_cast<uint64_t>(buffer.size())));
        if (bytesRead == 0) {
            break;
        }

        if (size < bytesRead) {
            bytesRead = size;
        }
        
        stream.write(reinterpret_cast<char*>(buffer.data()), bytesRead);
        size -= bytesRead;          
    }
    
    return totalSize - size;
}

uint64_t SdlFile::Write(std::istream& stream, uint64_t size, uint64_t bufferSize) {
    uint64_t totalSize = size;
    std::vector<uint8_t> buffer(bufferSize);
    while (size > 0) {
        stream.read(reinterpret_cast<char*>(buffer.data()), std::min(size, static_cast<uint64_t>(buffer.size())));
        uint64_t bytesRead = stream.gcount();
        if (bytesRead == 0) {
            break;
        }
        
        if (size < bytesRead) {
            bytesRead = size;
        }
        
        Write(buffer.data(), bytesRead);
        size -= bytesRead;
    }
    
    return totalSize - size;
}

// --- SdlFileSystem ---

SdlFileSystem::SdlFileSystem(const std::string& basePath)
    : m_basePath(basePath)
{}

SdlFileSystem::~SdlFileSystem() {
    SdlFileSystem::Shutdown();
}

void SdlFileSystem::Initialize() {
    m_isInitialized = true;
}

void SdlFileSystem::Shutdown() {
    m_isInitialized = false;
    m_fileList.clear();
}

bool SdlFileSystem::IsInitialized() const {
    return m_isInitialized;
}

const std::string& SdlFileSystem::BasePath() const {
    return m_basePath;
}

const vfspp::IFileSystem::TFileList& SdlFileSystem::FileList() const {
    return m_fileList;
}

bool SdlFileSystem::IsReadOnly() const {
    std::error_code ec;
    auto perms = std::filesystem::status(m_basePath, ec).permissions();
    if (ec) {
        // Если путь не распознается (например, assets из Android APK), 
        // безопасно считать файловую систему Read-Only.
        return true; 
    }
    return (perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none;
}

vfspp::IFilePtr SdlFileSystem::OpenFile(const vfspp::FileInfo& filePath, vfspp::IFile::FileMode mode) {
    auto file = std::make_shared<SdlFile>(filePath);
    file->Open(mode);
    if (!file->IsOpened()) {
        return nullptr;
    }
    return file;
}

bool SdlFileSystem::CreateFile(const vfspp::FileInfo& filePath) {
    if (IsReadOnly()) return false;

    SDL_RWops* rw = SDL_RWFromFile(filePath.AbsolutePath().c_str(), "wb");
    if (rw) {
        SDL_RWclose(rw);
        return true;
    }
    return false;
}

bool SdlFileSystem::RemoveFile(const vfspp::FileInfo& filePath) {
    if (IsReadOnly()) return false;
    std::error_code ec;
    return std::filesystem::remove(filePath.AbsolutePath(), ec);
}

bool SdlFileSystem::CopyFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) {
    if (IsReadOnly()) return false;
    std::error_code ec;
    return std::filesystem::copy_file(src.AbsolutePath(), dest.AbsolutePath(), std::filesystem::copy_options::overwrite_existing, ec);
}

bool SdlFileSystem::RenameFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) {
    if (IsReadOnly()) return false;
    std::error_code ec;
    std::filesystem::rename(src.AbsolutePath(), dest.AbsolutePath(), ec);
    return !ec;
}

bool SdlFileSystem::IsFileExists(const vfspp::FileInfo& filePath) const {
    std::error_code ec;
    if (std::filesystem::exists(filePath.AbsolutePath(), ec)) {
        return true;
    }

    // Fallback для SDL-специфичных путей (Android apk assets)
    SDL_RWops* rwOps = SDL_RWFromFile(filePath.AbsolutePath().c_str(), "rb");
    if (rwOps) {
        SDL_RWclose(rwOps);
        return true;
    }
    return false;
}

bool SdlFileSystem::IsFile(const vfspp::FileInfo& filePath) const {
    std::error_code ec;
    if (std::filesystem::is_regular_file(filePath.AbsolutePath(), ec)) {
        return true;
    }
    // Fallback для SDL assets
    return IsFileExists(filePath); 
}

bool SdlFileSystem::IsDir(const vfspp::FileInfo& dirPath) const {
    std::error_code ec;
    bool isDirectory = std::filesystem::is_directory(dirPath.AbsolutePath(), ec);
    if (!ec) {
        return isDirectory;
    }

    // На SDL / Android assets нет стандартного простого способа проверить директорию без 
    // дополнительных JNI вызовов к AssetManager, поэтому оставляем false.
    return false;
}

}
