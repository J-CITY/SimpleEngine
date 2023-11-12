#pragma once

#include <string>
#include "resourceManager.h"
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
			static ResourcePtr<RENDER::ShaderInterface> Create(const std::string& filePath);
			static ResourcePtr<RENDER::ShaderInterface> CreateFromResource(const RENDER::ShaderResource& res);

			static ResourcePtr<RENDER::ShaderInterface> CreateFromSource(const std::string& vertexShader, const std::string& fragmentShader,
				const std::string& geometryShader="", const std::string& tessCompShader = "", const std::string& tessEvoluationShader = "", const std::string& computeShader = "");
			static void Recompile(RENDER::ShaderInterface& shader);

		protected:
			static ResourcePtr<RENDER::ShaderInterface> CreateWithEmptyDeleter(const std::string& filePath);

			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path) override;
			
			//for error print
			static std::string FILE_PATH;
			static void UpdateFileWatchResource(const std::string& filePath);
			static void AddFileWatchSubscribe(const RENDER::ShaderResource& _res, const std::string& filePath);
			inline static std::unordered_map<std::string, std::vector<IKIGAI::ObjectIdGenerator<EVENT::Event<>>::id>> fwSubscribersIds;
		};

	}
}
