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
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path) override;
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path, ELoadingType type) override;
			virtual std::shared_ptr<RENDER::ShaderInterface> createResource(const std::string& path, ELoadingType type, std::any data) override;
			
			//for error print
			inline static std::string FILE_PATH;

			static void UpdateFileWatchResource(const std::string& filePath, const RENDER::ShaderResource& res, std::weak_ptr<RENDER::ShaderInterface> weakShader);
			static void AddFileWatchSubscribe(const RENDER::ShaderResource& _res, const std::string& filePath, std::weak_ptr<RENDER::ShaderInterface> weakShader);
			inline static std::unordered_map<std::string, std::vector<IKIGAI::IdGenerator<EVENT::Event<>>::id>> fwSubscribersIds;
		};

	}
}
