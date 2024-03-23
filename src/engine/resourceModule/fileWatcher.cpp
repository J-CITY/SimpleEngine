#include "fileWatcher.h"

using namespace IKIGAI::RESOURCES;

IKIGAI::IdGenerator<IKIGAI::EVENT::Event<>>::id FileWatcher::_add(const std::string& path, std::function<void(FileStatus)> cb) {
	if (!std::filesystem::exists(path)) {
		throw;
	}
	m_filesLastModifications[path] = std::filesystem::last_write_time(path);
	if (!m_filesCallbacks.contains(path)) {
		m_filesCallbacks[path] = EVENT::Event<FileStatus>();
	}
	return m_filesCallbacks[path].add(cb);
}

IKIGAI::IdGenerator<IKIGAI::EVENT::Event<>>::id FileWatcher::add(const std::filesystem::path& path, std::function<void(FileStatus)> cb) {
	const std::lock_guard lock(m_mutex);
	const auto _path = path.string();
	return _add(_path, cb);
}

void FileWatcher::addDeferred(const std::filesystem::path& path, std::function<void(FileStatus)> cb, std::function<void(EVENT::Event<FileStatus>::id)> retCb) {
	const std::lock_guard lock(m_mutexDeferred);
	if (!std::filesystem::exists(path)) {
		throw;
	}
	const auto _path = path.string();

	deferredEvents.push({ QueueEvent::Action::ADD, _path, cb, retCb });
}

void FileWatcher::_remove(const std::string& path, EVENT::Event<FileStatus>::id id) {
	if (!m_filesCallbacks.contains(path)) {
		return;
	}
	m_filesCallbacks.at(path).removeListener(id);
	if (!m_filesCallbacks.at(path).getListenerCount()) {
		m_filesCallbacks.erase(path);
		m_filesLastModifications.erase(path);
	}
}

void FileWatcher::remove(const std::filesystem::path& path, EVENT::Event<FileStatus>::id id) {
	const std::lock_guard lock(m_mutex);
	const auto _path = path.string();
	_remove(_path, id);
}

void FileWatcher::removeDeferred(const std::filesystem::path& path, EVENT::Event<FileStatus>::id id) {
	const std::lock_guard lock(m_mutexDeferred);
	if (!std::filesystem::exists(path)) {
		throw;
	}
	const auto _path = path.string();

	deferredEvents.push({ QueueEvent::Action::REMOVE, _path, nullptr, nullptr, id });
	return;
}

void FileWatcher::start() {
	m_isRunning = true;
}

void FileWatcher::stop() {
	m_isRunning = false;
}

void FileWatcher::applyUpdate() {
	{
		const std::lock_guard lock(m_mutex);
		for (auto& e : m_events) {
			m_filesCallbacks[e.first].run(e.second);
		}
		m_events.clear();
	}
	{
		const std::lock_guard lock(m_mutexDeferred);
		while (!deferredEvents.empty()) {
			auto e = deferredEvents.front();
			deferredEvents.pop();

			if (e.action == QueueEvent::Action::ADD) {
				e.retCb(_add(e.path, e.cb));
			}
			else {
				_remove(e.path, e.id.value());
			}
		}
	}

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
