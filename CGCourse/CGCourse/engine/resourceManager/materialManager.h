#pragma once
#include <string>
#include "resourceManager.h"

namespace KUMA::RENDER {
	class Material;
}

namespace KUMA {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::Material> {
		public:
			static ResourcePtr<RENDER::Material> CreateFromFile(const std::string& path);
			//move to private
			static ResourcePtr<RENDER::Material> Create(const std::string& path);
		protected:
			virtual ResourcePtr<RENDER::Material> createResource(const std::string& path) override;
		};
	}
}
