#pragma once
#include <string>

#include "resourceManager.h"
#include "ServiceManager.h"
#include "../render/Material.h"
#include <fstream>

namespace KUMA {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::Material> {
		public:
			static ResourcePtr<RENDER::Material> CreateFromFile(const std::string& path) {
				const std::string realPath = getRealPath(path);
				auto material = Create(realPath);
				if (material) {
					material->path = path;
				}
				return material;
			}

			//move to private
			static ResourcePtr<RENDER::Material> Create(const std::string& path) {
				auto material = ResourcePtr<RENDER::Material>(new RENDER::Material(), [](RENDER::Material* m) {
					ServiceManager::Get<MaterialLoader>().unloadResource<MaterialLoader>(m->path);
				});
				if (!path.empty()) {
					std::ifstream ifs(path);
					auto root = nlohmann::json::parse(ifs);
					material->onDeserialize(root);
				}
				return material;
			}
		protected:
			virtual ResourcePtr<RENDER::Material> createResource(const std::string& path) override {
				return CreateFromFile(path);
			}
		};
	}
}
