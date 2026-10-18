#pragma once
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

namespace IKIGAI::RESOURCES::STORAGE {
// SDL operations use short-lived containers. All names are storage-relative UTF-8.
class UserStorage {
	std::filesystem::path mRoot;
	std::string mOrganization, mApplication;
	mutable std::mutex mMutex;
public:
	struct Entry { std::string name; bool directory; };
	explicit UserStorage(std::filesystem::path root);
	UserStorage(std::string organization, std::string application);
	const std::filesystem::path& root() const { return mRoot; }
	std::vector<uint8_t> read(const std::string& path, uint64_t limit = 64 * 1024 * 1024) const;
	void write(const std::string& path, std::span<const uint8_t> data);
	void writeAtomic(const std::string& path, std::span<const uint8_t> data);
	bool exists(const std::string& path) const;
	bool isDirectory(const std::string& path) const;
	std::vector<Entry> list(const std::string& path = "") const;
	void createDirectories(const std::string& path);
	void remove(const std::string& path);
	// Native: already complete. Web: resolves only after IndexedDB sync finishes.
	std::shared_future<void> commit() const;
};
std::shared_future<void> ReadyCommit();
std::shared_future<void> CommitPersistent();
}
