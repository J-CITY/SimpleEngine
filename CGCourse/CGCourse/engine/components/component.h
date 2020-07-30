#pragma once
#include <chrono>


#include "../Shader.h"

namespace SE {
	class Object;

	enum class ComponentType {
		CUSTOM,
		TRANSFORM,
		
	};
	
	class Component {
	public:
		Component() = default;
		virtual ~Component() {};

		virtual void update() {};
		virtual void render(std::shared_ptr<Shader> shader) {};

		const ComponentType getType() const {
			return type;
		}
		void setParent(Object* obj) {
			object = obj;
		}
		Object* getParent(void) const {
			return object;
		}
	protected:
		Object* object = nullptr;
		ComponentType type;
	};
};
