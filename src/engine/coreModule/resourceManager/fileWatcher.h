#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <atomic>
#include <unordered_map>
#include <utilsModule/event.h>

namespace IKIGAI::RESOURCES {
	class FileWatcher {
	public:
		constexpr static int DELAY_UPDATE = 5000;
		enum class FileStatus { CREATE, MODIFIED, DEL };
	private:

		std::unordered_map<std::string, std::filesystem::file_time_type> m_filesLastModifications;
		std::unordered_map<std::string, EVENT::Event<FileStatus>> m_filesCallbacks;
		std::vector<std::pair<std::string, FileStatus>> m_events;

		std::atomic_bool m_isRunning = true;
		std::mutex m_mutex;
		std::chrono::duration<int, std::milli> m_delay = std::chrono::milliseconds(DELAY_UPDATE);
	public:
		inline static FileWatcher* m_instance = nullptr;
		static FileWatcher* getInstance() {
			if (!m_instance) {
				m_instance = new FileWatcher();
			}
			return m_instance;
		}

		EVENT::Event<FileStatus>::id add(const std::filesystem::path& path, std::function<void(FileStatus)> cb);
		void remove(const std::filesystem::path& path, EVENT::Event<FileStatus>::id id);

		void start();
		void stop();

		void applyUpdate();
		void update();
	};
}
