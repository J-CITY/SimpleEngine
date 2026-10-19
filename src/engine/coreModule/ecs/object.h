#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>


#include "componentManager.h"
#include "components/scriptComponent.h"
#include "components/transform.h"
#include "ecsModule/world.h"
#include <scriptModule/scriptInterpreter.h>
#include <utilsModule/event.h>


namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}

namespace IKIGAI::ECS {
class Object : public std::enable_shared_from_this<Object> {
public:
	struct Descriptor {
		std::string Name;
		std::string Tag;
		int Id;
		int ParentId;
		bool IsActive = false;
		bool IsVisible = true;
		std::vector<ComponentsDescriptorType> Components;

		template <class Context> constexpr static auto serde(Context& context, Descriptor& value) {
			using Self = Descriptor;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::Name, "Name")
				.field(&Self::Tag, "Tag", default_ { "" })
				.field(&Self::Id, "Id")
				.field(&Self::ParentId, "ParentId", default_ { -1 })
				.field(&Self::IsActive, "IsActive", default_ { true })
				.field(&Self::IsVisible, "IsVisible", default_ { true })
				.field(&Self::Components, "Components");
		}
	};

	Descriptor getDescriptor() const;

	// using Id_ = Id<Object>;
	using Id = ECS2::Entity;
	using ObjPtr = std::shared_ptr<Object>;

	Object(ECS2::Entity actorID, const std::string& name, const std::string& tag, ECS2::World* world = nullptr);
	explicit Object(const Descriptor& actorID, ECS2::World* world = nullptr, bool deferred = false);
	ECS2::World& getWorld() const {
		return *mWorld;
	}
	void addSerializedComponent(const ComponentsDescriptorType& descriptor);
	void setPendingActive(bool active) {
		mIsActive = active;
	}
	void activateLoaded();
	~Object();
	void dispose();

	[[nodiscard]] const std::string& getName() const;
	[[nodiscard]] const std::string& getTag() const;

	void setName(const std::string& name);
	void setTag(const std::string& tag);
	void setActive(bool val);
	void setVisible(bool val);

	[[nodiscard]] bool getIsSelfActive() const;
	[[nodiscard]] bool getIsActive() const;

	[[nodiscard]] bool getIsSelfVisible() const;
	[[nodiscard]] bool getIsVisible() const;

	void setID(ECS2::Entity id);
	[[nodiscard]] ECS2::Entity getID() const;

	void setParent(ObjPtr p_parent);
	void setParentInPos(std::shared_ptr<Object> _parent, int pos);

	void detachFromParent();

	[[nodiscard]] bool hasParent() const;
	[[nodiscard]] ObjPtr getParent() const;
	[[nodiscard]] ECS2::Entity getParentID() const;
	std::span<ObjPtr> getChildren();

	void markAsDestroy();

	bool isAlive() const;

	void onStart();
	void onEnable();
	void onDisable();
	void onDestroy();

	void onUpdate(std::chrono::duration<double> dt);
	void onFixedUpdate(std::chrono::duration<double> dt);
	void onLateUpdate(std::chrono::duration<double> dt);

	template <typename T, typename... Args> inline UTILS::WeakPtr<T> addComponent(Args&&... args) {
		if (mDisposed || mDisposing)
			throw std::logic_error("Cannot add a component to a disposed object");
		static_assert(std::is_base_of_v<ComponentBase, T>, "T should derive from Component");

		if (auto found = getComponent<T>(); !found) {
			auto instance = T(*this, args...);
			if (!mDeferred && getIsActive()) {
				// TODO: call systems methods
				instance.onAwake();
				instance.onEnable();
				instance.onStart();
			}
			// ComponentManager::GetInstance().addComponent<T>(getID(), instance);
			getWorld().getComponentManager()->addComponent<T>(getID(), std::move(instance));

			auto component = getComponent<T>();
			componentAddedEvent.run(component);
			// TODO: move it to ScriptComponent::onStart()
#if USE_LUA

			if constexpr (std::is_same_v<T, ScriptComponent>) {
				if (!mDeferred)
					ScriptComponentEvents::createdEvent.run(component);
			}
#endif
			mComponents[std::type_index(typeid(T))] = component;
			return component;
		}
		else {
			return found;
		}
	}

	template <typename T> inline bool removeComponent() {
		static_assert(std::is_base_of_v<ComponentBase, T>, "T should derive from Component");
		// create method in rteflection genereted file
		// static_assert(!std::is_same_v<TransformComponent, T>, "You can't remove a Transform from an actor");
		if constexpr (std::is_same<T, TransformComponent>::value) {
			return false;
		}

		auto result = getComponent<T>();
		if (!result) {
			return false;
		}
		// TODO: move it to ScriptComponent::onStart()
#if USE_LUA

		if constexpr (std::is_same<T, ScriptComponent>::value) {
			ScriptComponentEvents::destroyedEvent.run(result);
		}
#endif
#if USE_IKIGAI_SCRIPT
		if constexpr (std::is_same_v<T, IkigaiScriptComponent>)
			result->onDestroy();
#endif
		componentRemovedEvent.run(result);
		// ComponentManager::GetInstance().removeComponent<T>(getID());
		getWorld().getComponentManager()->removeComponents<T>(getID());
		mComponents.erase(std::type_index(typeid(T)));
		return true;
	}

	template <typename T> inline UTILS::WeakPtr<T> getComponent() const {
		if (mDisposed)
			return nullptr;
		static_assert(std::is_base_of_v<ComponentBase, T>, "T should derive from Component");

		// if (ComponentManager::GetInstance().checkComponent<T>(getID())) {
		//	return ComponentManager::GetInstance().getComponent<T>(getID());
		// }
		if (getWorld().getComponentManager()->checkComponent<T>(getID())) {
			return getWorld().getComponentManager()->getComponent<T>(getID());
		}
		return nullptr;
	}

	template <typename T> inline UTILS::WeakPtr<ComponentBase> getComponentBase() const {
		if (mDisposed)
			return nullptr;
		static_assert(std::is_base_of_v<ComponentBase, T>, "T should derive from Component");

		// if (ComponentManager::GetInstance().checkComponent<T>(getID())) {
		//	return ComponentManager::GetInstance().getComponentBase<T>(getID());
		// }
		if (getWorld().getComponentManager()->checkComponent<T>(getID())) {
			return getWorld().getComponentManager()->getComponentBase<T>(getID());
		}

		return nullptr;
	}

	auto getComponents() const {
		// return ComponentManager::GetInstance().getComponents(getID());
		// return getWorld().getComponentManager()->getComponents(getID());
		return mComponents;
	}

	[[nodiscard]] UTILS::WeakPtr<TransformComponent> getTransform() const;

	// ObjectData getObjectData();

private:
	void recursiveActiveUpdate();

public:
	EVENT::Event<UTILS::WeakPtr<ComponentBase>> componentAddedEvent;
	EVENT::Event<UTILS::WeakPtr<ComponentBase>> componentRemovedEvent;
	EVENT::Event<UTILS::WeakPtr<ComponentBase>> componentChangedEvent;

	static EVENT::Event<Object&> destroyedEvent;
	static EVENT::Event<Object&> createdEvent;
	static EVENT::Event<Object&, Object&> attachEvent;
	static EVENT::Event<Object&> dettachEvent;

	// TODO: think how do it better
	EVENT::Event<> setModelEvent;

	// for serealization
	int getIDInt() const;
	void setIDInt(int _id);
	int getParentId() const;
	void setParentId(int _id);

protected:
	ECS2::World* mWorld = nullptr;
	bool mDeferred = false;
	bool mDisposed = false;
	bool mDisposing = false;
	// Id_ id;
	ECS2::Entity mEntity;
	std::string mName;
	std::string mTag;

	bool mIsActive = true;
	bool mIsVisible = true;
	bool mIsDestroyed = false;
	bool mIsReady = false;

	std::weak_ptr<Object> mParent;
	std::vector<ObjPtr> mChildren;
	std::map<std::type_index, UTILS::WeakPtr<ComponentBase>> mComponents;

public:
	UTILS::WeakPtr<TransformComponent> mTransform;
};
};
