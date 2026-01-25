#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "entityManager.h"
#include "signature.h"
#include "componentManager.h"
#include "access.h"
#include "commandBuffer.h"

namespace IKIGAI::ECS2 {
	
	class World;

	// Access Types for dependency graph
	// Defined in access.h

	class System {
	public:
		virtual ~System() = default;
		
		// Lifecycle methods
		virtual void onAwake(World& world) {}
		virtual void onStart(World& world) {}
		virtual void onDestroy(World& world) {}
		
		// Update phases with delta time
		virtual void onUpdate(World& world, CommandBuffer& cb, double dt) {}
		virtual void onFixedUpdate(World& world, CommandBuffer& cb, double dt) {}
		virtual void onLateUpdate(World& world, CommandBuffer& cb, double dt) {}
		
		// Legacy immediate mode (deprecated, use onUpdate instead)
		virtual void run(World& world, CommandBuffer& cb) {}
		
		// Metadata for Graph
		std::string mName;
		std::unordered_set<ComponentType> mReads;
		std::unordered_set<ComponentType> mWrites;
		
		// Signature for matching (if Query based)
		Signature mQueryHas;
		Signature mQueryNot;
		
		// State flags
		bool mIsAwake = false;
		bool mIsStarted = false;
	};

	class SystemManager {
	public:
		struct Batch {
			std::vector<std::shared_ptr<System>> systems;
		};

		SystemManager(World& world) : mWorld(world) {}

		template<typename T, typename... Args>
		std::shared_ptr<T> registerSystem(Args&&... args) {
			auto system = std::make_shared<T>(std::forward<Args>(args)...);
			mSystems.push_back(system);
			return system;
		}

		void addSystem(std::shared_ptr<System> system) {
			mSystems.push_back(system);
			mIsDirty = true;
		}

		[[nodiscard]] ComponentManager* getComponentManager() const noexcept;

		// Phase execution methods
		void runAwake();
		void runStart();
		void runUpdate(double dt);
		void runFixedUpdate(double dt);
		void runLateUpdate(double dt);
		void runDestroy();
		
		// Legacy immediate mode (deprecated)
		void run();
		
		World& getWorld() const;

	private:

		World& mWorld;
		std::vector<std::shared_ptr<System>> mSystems;
		std::vector<Batch> mBatches;
		bool mIsDirty = true;

		void rebuildBatches();
		
		// Dependency Graph
		// std::vector<std::vector<int>> mGraph;
		// void buildGraph();
	};
}
