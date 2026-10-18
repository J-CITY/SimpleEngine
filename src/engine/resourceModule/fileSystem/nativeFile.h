#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vfspp/IFile.h>

namespace IKIGAI::RESOURCES {
// std::filesystem::path preserves Unicode on Windows; vfspp's std::fopen path
// uses the ANSI code page. Keep this adapter in the engine rather than patching
// the dependency. Mount policy still decides whether writes are permitted.
class NativeDiskFile final : public vfspp::IFile {
	vfspp::EntryInfo mEntry;
	std::fstream mStream;
	FileMode mMode = FileMode::Read;
	uint64_t mPosition = 0;

public:
	explicit NativeDiskFile(vfspp::EntryInfo entry)
		: mEntry(std::move(entry)) { }
	const vfspp::EntryInfo& GetEntryInfo() const override {
		return mEntry;
	}
	uint64_t Size() const override {
		std::error_code ec;
		const auto size = std::filesystem::file_size(std::filesystem::u8path(mEntry.NativePath()), ec);
		return ec ? 0 : size;
	}
	bool IsReadOnly() const override {
		return !ModeHasFlag(mMode, FileMode::Write);
	}
	bool Open(FileMode mode) override {
		Close();
		if (!IsModeValid(mode))
			return false;
		mMode = mode;
		auto flags = std::ios::binary;
		const auto path = std::filesystem::u8path(mEntry.NativePath());
		if (ModeHasFlag(mode, FileMode::Read))
			flags |= std::ios::in;
		if (ModeHasFlag(mode, FileMode::Write)) {
			flags |= std::ios::out;
			if (ModeHasFlag(mode, FileMode::Append))
				flags |= std::ios::app;
			else if (ModeHasFlag(mode, FileMode::Truncate) || !std::filesystem::exists(path))
				flags |= std::ios::trunc;
			else
				flags |= std::ios::in;
		}
		mStream.clear();
		mStream.open(path, flags);
		mPosition = ModeHasFlag(mode, FileMode::Append) ? Size() : 0;
		return mStream.is_open();
	}
	void Close() override {
		if (mStream.is_open())
			mStream.close();
		mPosition = 0;
	}
	bool IsOpened() const override {
		return mStream.is_open();
	}
	uint64_t Seek(uint64_t offset, Origin origin) override {
		uint64_t position = offset;
		if (origin == Origin::End)
			position = offset <= Size() ? Size() - offset : 0;
		else if (origin == Origin::Set)
			position = mPosition + offset;
		mStream.clear();
		if (ModeHasFlag(mMode, FileMode::Read))
			mStream.seekg(static_cast<std::streamoff>(position));
		if (ModeHasFlag(mMode, FileMode::Write))
			mStream.seekp(static_cast<std::streamoff>(position));
		if (mStream)
			mPosition = position;
		return mPosition;
	}
	uint64_t Tell() const override {
		return mPosition;
	}
	uint64_t Read(std::span<uint8_t> bytes) override {
		if (!IsOpened() || !ModeHasFlag(mMode, FileMode::Read))
			return 0;
		mStream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		const auto count = static_cast<uint64_t>(mStream.gcount());
		mPosition += count;
		return count;
	}
	uint64_t Read(std::vector<uint8_t>& bytes, uint64_t size) override {
		bytes.resize(static_cast<size_t>(std::min(size, Size() - std::min(Size(), mPosition))));
		const auto count = Read(std::span<uint8_t>(bytes));
		bytes.resize(static_cast<size_t>(count));
		return count;
	}
	uint64_t Write(std::span<const uint8_t> bytes) override {
		if (!IsOpened() || IsReadOnly())
			return 0;
		mStream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		if (!mStream)
			return 0;
		mStream.flush();
		if (!mStream)
			return 0;
		mPosition += bytes.size();
		return bytes.size();
	}
	uint64_t Write(const std::vector<uint8_t>& bytes) override {
		return Write(std::span<const uint8_t>(bytes));
	}
};
}
