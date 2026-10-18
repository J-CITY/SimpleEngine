#pragma once
#if USE_PHYSICS_3D
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
namespace IKIGAI::SCENE_SYSTEM {
class Scene;
}
namespace IKIGAI::ECS {
struct Physics3DLease;
}
namespace IKIGAI::RENDER {
class MeshInterface;
class ModelInterface;
// Owns GPU resources; simulation and ECS leases never own renderer objects.
class DeformableRenderer {
	struct Entry {
		std::weak_ptr<ECS::Physics3DLease> lease;
		std::shared_ptr<MeshInterface> mesh;
		std::shared_ptr<ModelInterface> model;
		std::size_t vertices = 0, indices = 0;
		std::vector<std::array<float, 2>> uv;
		std::uint64_t frame = 0;
	};
	std::unordered_map<const ECS::Physics3DLease*, Entry> mEntries;
	std::uint64_t mFrame = 0;

public:
	void update(SCENE_SYSTEM::Scene&);
};
}

#endif // USE_PHYSICS_3D
