#pragma once
#include <string>

#include "resourceManager.h"
#include "../render/Material.h"
#include <fstream>

namespace KUMA {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::Material> {
		public:
			static bool Destroy(std::shared_ptr<RENDER::Material> material) {
				if (material) {
					material.reset();
					return true;
				}
				return false;
			}
			static std::shared_ptr<RENDER::Material> CreateFromFile(const std::string& path) {
				std::string realPath = getRealPath(path);

				std::shared_ptr<RENDER::Material> material = MaterialLoader::Create(realPath);
				if (material) {
					material->path = path;
				}

				return material;
			}

			//move to private
			static std::shared_ptr<RENDER::Material> Create(const std::string& path) {
				std::shared_ptr<RENDER::Material> material = std::make_shared<RENDER::Material>();
				if (!path.empty()) {
					std::ifstream ifs(path);
					auto root = nlohmann::json::parse(ifs);
					material->onDeserialize(root);
				}
				return material;
			}
		protected:
			
			virtual std::shared_ptr<RENDER::Material> createResource(const std::string& path) override {
				return CreateFromFile(path);
			}

			virtual void destroyResource(std::shared_ptr<RENDER::Material> res) override {
				Destroy(res);
			}
		};
	}
}
