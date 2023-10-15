#pragma once
#include <string>
#include "resourceManager.h"

namespace IKIGAI::RENDER {
	class MaterialInterface;
}

namespace IKIGAI {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::MaterialInterface> {
		public:
			static ResourcePtr<RENDER::MaterialInterface> CreateFromFile(const std::string& path);
			//move to private
			static ResourcePtr<RENDER::MaterialInterface> Create(const std::string& path);
		protected:
			virtual ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path) override;
		};
	}
}