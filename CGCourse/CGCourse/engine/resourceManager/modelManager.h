#pragma once
#include <string>

#include "resourceManager.h"
#include "parser/assimpParser.h"

namespace KUMA
{
	namespace RENDER
	{
		class ModelInterface;
	}
}

namespace KUMA {
	namespace RESOURCES {
		class ModelLoader : public ResourceManager<RENDER::ModelInterface> {
		public:
			static void Reload(RENDER::ModelInterface& model, const std::string& filePath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
			static ResourcePtr<RENDER::ModelInterface> CreateFromFile(const std::string& path);
			static ResourcePtr<RENDER::ModelInterface> CreateFromFile(const std::string& path, ModelParserFlags parserFlags);
			//move to private
			static ResourcePtr<RENDER::ModelInterface> Create(const std::string& filepath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
		protected:
			static AssimpParser _ASSIMP;

			//TODO: load from file
			static ModelParserFlags getAssetMetadata(const std::string& path);

			virtual ResourcePtr<RENDER::ModelInterface> createResource(const std::string& path) override;
		};
	
	}
}
