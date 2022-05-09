#pragma once
#include <string>

#include "resourceManager.h"
#include "../render/Material.h"

namespace KUMA {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::Material> {
		public:
			static std::shared_ptr<RENDER::Material> Create(const std::string& path) {
				std::shared_ptr<RENDER::Material> material = std::make_shared<RENDER::Material>();

				//material->OnDeserialize(doc, root);
				return material;
			}
			
			static bool Destroy(std::shared_ptr<RENDER::Material> material) {
				if (material) {
					material.reset();
					return true;
				}
				return false;
			}
		public:
			virtual std::shared_ptr<RENDER::Material> createResource(const std::string& path) override {
				std::string realPath = getRealPath(path);

				std::shared_ptr<RENDER::Material> material = MaterialLoader::Create(realPath);
				if (material) {
					material->path = path;
				}

				return material;
			}
			virtual void destroyResource(std::shared_ptr<RENDER::Material> res) override {
				MaterialLoader::Destroy(res);
			}

		};
	}
}
