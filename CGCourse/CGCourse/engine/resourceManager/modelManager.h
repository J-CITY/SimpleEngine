#pragma once
#include <string>

#include "resourceManager.h"
#include "parser/assimpParser.h"

namespace KUMA {
	namespace RESOURCES {
		class ModelLoader : public ResourceManager<RENDER::Model> {
		public:
			static void Reload(RENDER::Model& model, const std::string& filePath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
			static ResourcePtr<RENDER::Model> CreateFromFile(const std::string& path);
			static ResourcePtr<RENDER::Model> CreateFromFile(const std::string& path, ModelParserFlags parserFlags);
			//move to private
			static ResourcePtr<RENDER::Model> Create(const std::string& filepath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
		protected:
			static AssimpParser _ASSIMP;

			//TODO: load from file
			static ModelParserFlags getAssetMetadata(const std::string& path);

			virtual ResourcePtr<RENDER::Model> createResource(const std::string& path) override;
		};
	
	}
}
