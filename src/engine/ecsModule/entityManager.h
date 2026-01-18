#pragma once
#include "utilsModule/idGenerator.h"

namespace IKIGAI::ECS2 {
	class Entity: public IdGenerator<Entity> {
	public:
		Entity() = default;

		bool operator==(const IKIGAI::ECS2::Entity& rhs) const noexcept {
			return getUniqueId() == rhs.getUniqueId();
		}
	};
}

template <>
struct std::hash<IKIGAI::ECS2::Entity> {
	std::size_t operator()(const IKIGAI::ECS2::Entity& e) const {
		return static_cast<int>(e.getUniqueId());
	}
};