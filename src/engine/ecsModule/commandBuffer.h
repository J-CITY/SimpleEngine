#pragma once
#include <vector>
#include <functional>
#include <mutex>
#include "entityManager.h"

namespace IKIGAI::ECS2 {
	class CommandBuffer {
	public:
		void addCommand(std::function<void()>&& cmd) {
			mCommands.push_back(std::move(cmd));
		}

		void execute() {
			for (auto& cmd : mCommands) {
				cmd();
			}
			mCommands.clear();
		}

	private:
		std::vector<std::function<void()>> mCommands;
	};
}
