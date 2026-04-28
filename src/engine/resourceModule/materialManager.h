#pragma once
#include <string>
#include "resourceManager.h"
#include "utilsModule/event.h"
#include "utilsModule/idGenerator.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "renderModule/backends/interface/driverInterface.h"

namespace IKIGAI::RENDER {
	class MaterialInterface;
}

namespace IKIGAI {
	namespace RESOURCES {
		class MaterialLoader : public ResourceManager<RENDER::MaterialInterface> {
		public:
			// Публичный API: только Create через путь к .material файлу
			static ResourcePtr<RENDER::MaterialInterface> Create(
				const std::string& path,
				UTILS::IAllocator* allocator = nullptr,
				RENDER::MaterialDeleter deleter = nullptr);

		private:
			// Вспомогательные: с кэшем и filewatcher
			ResourcePtr<RENDER::MaterialInterface> createFromResource(const std::string& path);

			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path) override;
			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path, ELoadingType type) override;
			ResourcePtr<RENDER::MaterialInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;

			static void AddToFileWatch(
				const std::string& watchPath,
				std::weak_ptr<RENDER::MaterialInterface> weakMat);

			RENDER::MaterialDeleter createCacheDeleter(const std::string& path);

			// Кэш разобранных дескрипторов материалов
			inline static std::unordered_map<std::string, RENDER::MaterialResource> sResourceCache;
			// FW subscription ids: path -> list of watch ids
			inline static std::unordered_map<std::string, std::vector<IKIGAI::IdGenerator<EVENT::Event<>>::id>> sFWSubscribersIds;
		};
	}
}
