#pragma once
#include <string>

#include "resourceManager.h"
#include "parser/assimpParser.h"

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
			static ResourcePtr<RENDER::ModelInterface> CreateFromFile(const std::string& path);
			static ResourcePtr<RENDER::ModelInterface> CreateFromResource(const std::string& path);
			static ResourcePtr<RENDER::ModelInterface> CreateFromFile(const std::string& path, ModelParserFlags parserFlags);
			//move to private
			static ResourcePtr<RENDER::ModelInterface> Create(const std::string& filepath, ModelParserFlags parserFlags = ModelParserFlags::NONE);
			static ResourcePtr<IKIGAI::RENDER::ModelInterface> CreateVerts(const std::string& filepath,
			                                                        ModelParserFlags parserFlags,
			                                                        std::vector<std::vector<Vertex>>&
			                                                        _globalVerticesPerMesh,
			                                                        std::vector<std::vector<uint32_t>>&
			                                                        _globalIndicesPerMesh);
			static ModelParserFlags getDefaultFlag();
		protected:
			static AssimpParser _ASSIMP;

			//TODO: load from file
			

			virtual ResourcePtr<RENDER::ModelInterface> createResource(const std::string& path) override;
		};
	
	}
}
