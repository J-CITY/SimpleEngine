#pragma once
#include <vector>
#include <assimp/scene.h>
#include <renderModule/backends/interface/modelInterface.h>

#include "parseFlags.h"
//#include "../resourceManager.h"

namespace IKIGAI::RESOURCES
{
	template<typename T>
	using ResourcePtr = std::shared_ptr<T>;

}

namespace IKIGAI {
	class Vertex;

	namespace RENDER {
		class Model;
	}
	namespace RESOURCES {
		class AssimpParser {
		public:
			bool LoadModel(const std::string& fileName, RESOURCES::ResourcePtr<RENDER::ModelInterface> model, ModelParserFlags parserFlags);
			bool LoadVertexes(const std::string& fileName, RESOURCES::ResourcePtr<RENDER::ModelInterface> model, ModelParserFlags parserFlags,
				std::vector<std::vector<Vertex>>& globalVerticesPerMesh,
				std::vector< std::vector<uint32_t>>& globalIndicesPerMesh);

		private:
			const unsigned int MAX_BONE_WEIGHTS = 4;
			void processMaterials(const struct aiScene* scene, std::vector<std::string>& materials);;
			void processNode(void* transform, struct aiNode* node, const struct aiScene* scene, RESOURCES::ResourcePtr<RENDER::ModelInterface> model);
			void processMesh(void* transform, struct aiMesh* mesh, const struct aiScene* scene, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
			void loadBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, RESOURCES::ResourcePtr<RENDER::ModelInterface> model);
			void setVertexBoneData(Vertex& vertex, int boneID, float weight);
		};
	}
}
