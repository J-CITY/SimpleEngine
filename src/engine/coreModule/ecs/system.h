#pragma once
#include <chrono>
#include <functional>
#include <set>
#include "componentArrayInterface.h"
#include <utilsModule/lockFreeQueue.h>


namespace IKIGAI::ECS {
	class System {
	public:
		enum class SystemComponentProperties {
			READ,
			WRITE
		};
		constexpr static auto SYSTEM_COMPONENTS_READ = SystemComponentProperties::READ;
		constexpr static auto SYSTEM_COMPONENTS_WRITE = SystemComponentProperties::WRITE;

		virtual ~System() = default;

		virtual void onAwake() {}
		virtual void onStart() {}

		virtual void onEnable() {}
		virtual void onDisable() {}

		virtual void onDestroy() {}

		virtual void onUpdate(std::chrono::duration<double> dt) {}
		virtual void onFixedUpdate(std::chrono::duration<double> dt) {}
		virtual void onLateUpdate(std::chrono::duration<double> dt) {}

		void delayed(std::function<void()> task);
		void onUpdateDelayed(std::chrono::duration<double> dt);

		std::set<Entity>& getEntities();
		[[nodiscard]] const std::set<Entity>& getEntities() const;

	private:
		std::set<Entity> m_entities;
		UTILS::LockFreeQueue<std::function<void()>> m_delayedTask;
	};

}
