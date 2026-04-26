#pragma once
#include <string>
#include "resourceManager.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"

namespace IKIGAI::RENDER {
	class MaterialInterface;
}

namespace IKIGAI {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::MaterialInterface> {
		public:
			static ResourcePtr<RENDER::MaterialInterface> CreateFromFile(const std::string& path);
			static ResourcePtr<RENDER::MaterialInterface> Create(const std::string& path);
		protected:
			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path) override;
			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path, ELoadingType type) override;
			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;

			inline static std::unordered_map<std::string, std::vector<IdGenerator<EVENT::Event<>>::id>> fwSubscribersIds;
		};
	}
}
