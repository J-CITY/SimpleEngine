#pragma once

#include <string>
#include "resourceManager.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/event.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class ShaderInterface;
	}
}

namespace IKIGAI {
	namespace RESOURCES {
		class ShaderLoader : public ResourceManager<RENDER::ShaderInterface> {
		public:
			static ResourcePtr<RENDER::ShaderInterface> Create(const std::string& filePath, UTILS::IAllocator* allocator = nullptr, RENDER::ShaderDeleter deleter = nullptr);
			static ResourcePtr<RENDER::ShaderInterface> CreateFromResource(const RENDER::ShaderResource& res, UTILS::IAllocator* allocator = nullptr, RENDER::ShaderDeleter deleter = nullptr);

			static ResourcePtr<RENDER::ShaderInterface> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
				const std::string& geometryShader="", const std::string& tessCompShader = "", const std::string& tessEvoluationShader = "", const std::string& computeShader = "",
				UTILS::IAllocator* allocator = nullptr, RENDER::ShaderDeleter deleter = nullptr);
			static void Recompile(RENDER::ShaderInterface& shader, const RENDER::ShaderResource& res);

		protected:
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path) override;
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path, ELoadingType type) override;
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;
			
			static void UpdateFileWatchResource(const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader);
			static void AddFileWatchSubscribe(const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader);
			inline static std::unordered_map<std::string, std::vector<std::pair<std::string, IKIGAI::IdGenerator<EVENT::Event<>>::ID>>> fwSubscribersIds;
			inline static std::unordered_map<std::string, RENDER::ShaderResource> sResourceCache;
		};
	}
}
