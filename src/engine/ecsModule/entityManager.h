#pragma once
#include "utilsModule/idGenerator.h"

namespace IKIGAI::ECS {
class Entity;
}
template <> struct IKIGAI::IdDomain<IKIGAI::ECS::Entity> {
	static constexpr std::uint32_t first = 1;
	static constexpr std::uint32_t last = std::numeric_limits<int>::max();
};

namespace IKIGAI::ECS {
class Entity : public IdGenerator<Entity> {
public:
	Entity() = default;
	Entity(ID id)
		: IdGenerator<IKIGAI::ECS::Entity>(id) { };

	bool operator==(const IKIGAI::ECS::Entity& rhs) const noexcept {
		return getUniqueId() == rhs.getUniqueId();
	}
};
}

template <> struct std::hash<IKIGAI::ECS::Entity> {
	std::size_t operator()(const IKIGAI::ECS::Entity& e) const {
		return std::hash<IKIGAI::Id<IKIGAI::ECS::Entity>> {}(e.getUniqueId());
	}
};
