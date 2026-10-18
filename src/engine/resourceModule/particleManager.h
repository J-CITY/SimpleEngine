#pragma once
#include "particleModule/particles.h"
#include "resourceManager.h"

namespace IKIGAI::RENDER {
class TextureInterface;
class MaterialInterface;
class MeshInterface;
}
namespace IKIGAI::RESOURCES {
struct ParticleAsset {
	struct DrawResources {
		std::string piece;
		std::shared_ptr<RENDER::TextureInterface> texture;
		std::shared_ptr<RENDER::MaterialInterface> material;
		std::shared_ptr<RENDER::MeshInterface> mesh;
	};
	std::shared_ptr<const PARTICLES::Definition> definition;
	std::vector<DrawResources> renderers;
};
class ParticleLoader : public ResourceManager<ParticleAsset, PARTICLES::Definition> {
public:
	ResourcePtr<ParticleAsset> createResource(const std::string& path) override;
	// Used by the legacy adapter; the immutable definition still shares the new runtime.
	ResourcePtr<ParticleAsset> createInline(std::shared_ptr<const PARTICLES::Definition> definition);

protected:
	COROUTINE::Task<ResourcePtr<ParticleAsset>> createResourceTask(
		std::string path, ELoadingType, std::shared_ptr<ResourceLoadControl> control) override;
	bool reloadResource(std::weak_ptr<ParticleAsset>, const std::string&) override {
		return false;
	}
};
}
