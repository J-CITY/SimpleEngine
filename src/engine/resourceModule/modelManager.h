#pragma once
#include <string>

#include "resourceManager.h"
#include "parser/assimpParser.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "utilsModule/event.h"
#include "renderModule/backends/interface/resourceStruct.h"
#include "utilsModule/memoryAlloc.h"

namespace IKIGAI
{
	namespace RENDER
	{
		class ModelInterface;
	}
}

namespace IKIGAI {
	namespace RESOURCES {
		class ModelLoader : public ResourceManager<RENDER::ModelInterface> {
		public:
			static void Reload(RENDER::ModelInterface& model, const std::string& filePath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
			static ResourcePtr<RENDER::ModelInterface> CreateFromResource(const std::string& path, UTILS::IAllocator* allocator = nullptr, RENDER::ModelDeleter deleter = nullptr);
			static ResourcePtr<RENDER::ModelInterface> CreateFromFile(const std::string& path, ModelParserFlags parserFlags, UTILS::IAllocator* allocator = nullptr, RENDER::ModelDeleter deleter = nullptr);
			//move to private
			static ResourcePtr<RENDER::ModelInterface> Create(const std::string& filepath, ModelParserFlags parserFlags = ModelParserFlags::NONE, UTILS::IAllocator* allocator = nullptr, RENDER::ModelDeleter deleter = nullptr);
			static ResourcePtr<IKIGAI::RENDER::ModelInterface> CreateVerts(const std::string& filepath,
			                                                        ModelParserFlags parserFlags,
			                                                        std::vector<std::vector<Vertex>>&
			                                                        _globalVerticesPerMesh,
			                                                        std::vector<std::vector<uint32_t>>&
			                                                        _globalIndicesPerMesh);
			static ModelParserFlags getDefaultFlag();

		private:
			static AssimpParser _ASSIMP;

			ResourcePtr<RENDER::ModelInterface> createResource(const std::string& path) override;
			ResourcePtr<RENDER::ModelInterface> createResource(const std::string& path, ELoadingType type) override;
			ResourcePtr<RENDER::ModelInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;

			// File watching
			static void UpdateFileWatchResource(const std::string& configPath, const std::string& meshPath, std::weak_ptr<RENDER::ModelInterface> weakModel, ModelParserFlags flags);
			static void AddFileWatchSubscribe(const std::string& configPath, const std::string& meshPath, std::weak_ptr<RENDER::ModelInterface> weakModel, ModelParserFlags flags);
			static void UnsubscribeFileWatch(const std::string& path);
			RENDER::ModelDeleter createCacheDeleter(const std::string& path);

			inline static std::unordered_map<std::string, std::vector<IKIGAI::IdGenerator<EVENT::Event<>>::id>> fwSubscribersIds;
			inline static std::unordered_map<std::string, RENDER::ModelResource> sResourceCache;
		};
	
	}
}
