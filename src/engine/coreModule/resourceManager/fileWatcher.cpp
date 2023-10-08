#include "fileWatcher.h"

using namespace IKIGAI::RESOURCES;

IKIGAI::ObjectIdGenerator<IKIGAI::EVENT::Event<>>::id FileWatcher::add(const std::filesystem::path& path, std::function<void(FileStatus)> cb) {
	const std::lock_guard lock(m_mutex);
	if (!std::filesystem::exists(path)) {
		throw;
	}
	const auto _path = path.string();
	m_filesLastModifications[_path] = std::filesystem::last_write_time(path);
	if (!m_filesCallbacks.contains(_path)) {
		m_filesCallbacks[_path] = EVENT::Event<FileStatus>();
	}
	return m_filesCallbacks[_path].add(cb);
}

void FileWatcher::remove(const std::filesystem::path& path, EVENT::Event<FileStatus>::id id) {
	const std::lock_guard lock(m_mutex);
	const auto _path = path.string();
	if (!m_filesCallbacks.contains(_path)) {
		return;
	}
	m_filesCallbacks.at(_path).removeListener(id);
	if (!m_filesCallbacks.at(_path).getListenerCount()) {
		m_filesCallbacks.erase(_path);
		m_filesLastModifications.erase(_path);
	}
}

void FileWatcher::start() {
	m_isRunning = true;
}

void FileWatcher::stop() {
	m_isRunning = false;
}

void FileWatcher::applyUpdate() {
	const std::lock_guard lock(m_mutex);
	for (auto& e : m_events) {
		m_filesCallbacks[e.first].run(e.second);
	}
	m_events.clear();
}

void FileWatcher::update() {
	if (!m_isRunning) {
		return;
	}
	const std::lock_guard lockm(m_mutex);
	for (auto& file : m_filesLastModifications) {
		if (!std::filesystem::exists(file.first)) {
			m_events.push_back({ file.first, FileStatus::DEL });
		}
		else {
			auto lastModification = std::filesystem::last_write_time(file.first);
			if (lastModification != file.second) {
				m_filesLastModifications[file.first] = lastModification;
				m_events.push_back({ file.first, FileStatus::MODIFIED });
			}
		}
	}
}
