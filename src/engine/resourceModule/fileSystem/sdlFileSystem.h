#pragma once

#include <vfspp/IFile.h>
#include <vfspp/IFileSystem.h>
#include <SDL.h>

namespace IKIGAI::RESOURCES {

    class SdlFile : public vfspp::IFile {
    public:
        SdlFile(const vfspp::FileInfo& fileInfo);
        ~SdlFile() override;

        const vfspp::FileInfo& GetFileInfo() const override;
        uint64_t Size() override;
        bool IsReadOnly() const override;
        void Open(FileMode mode) override;
        void Close() override;
        bool IsOpened() const override;
        uint64_t Seek(uint64_t offset, Origin origin) override;
        uint64_t Tell() override;
        uint64_t Read(uint8_t* buffer, uint64_t size) override;
        uint64_t Write(const uint8_t* buffer, uint64_t size) override;
        uint64_t Read(std::vector<uint8_t>& buffer, uint64_t size) override;
        uint64_t Write(const std::vector<uint8_t>& buffer) override;
        uint64_t Read(std::ostream& stream, uint64_t size, uint64_t bufferSize = 1024) override;
        uint64_t Write(std::istream& stream, uint64_t size, uint64_t bufferSize = 1024) override;

    private:
        std::string getSdlMode(FileMode mode) const;

        vfspp::FileInfo m_fileInfo;
        SDL_RWops* m_rwops = nullptr;
    };

    class SdlFileSystem : public vfspp::IFileSystem {
    public:
        SdlFileSystem(const std::string& basePath);
        ~SdlFileSystem();

        void Initialize() override;
        void Shutdown() override;
        bool IsInitialized() const override;
        const std::string& BasePath() const override;
        const TFileList& FileList() const override;
        bool IsReadOnly() const override;

        vfspp::IFilePtr OpenFile(const vfspp::FileInfo& filePath, vfspp::IFile::FileMode mode) override;
        bool CreateFile(const vfspp::FileInfo& filePath) override;
        bool RemoveFile(const vfspp::FileInfo& filePath) override;
        bool CopyFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) override;
        bool RenameFile(const vfspp::FileInfo& src, const vfspp::FileInfo& dest) override;
        bool IsFileExists(const vfspp::FileInfo& filePath) const override;
        bool IsFile(const vfspp::FileInfo& filePath) const override;
        bool IsDir(const vfspp::FileInfo& dirPath) const override;

    private:
        std::string m_basePath;
        bool m_isInitialized = false;
        TFileList m_fileList; // Usually empty since we can't easily iterate SDL_RWops
    };

}
