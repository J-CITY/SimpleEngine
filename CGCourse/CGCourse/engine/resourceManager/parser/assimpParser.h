#pragma once
#include <vector>
#include <assimp/scene.h>
import glmath;
#include "../resource/mesh.h"
#include "../resourceManager.h"
namespace KUMA {
	namespace RENDER {
		class Model;
	}
	namespace RESOURCES {
		enum class ModelParserFlags {
			NONE = 0x0,
			CALC_TANGENT_SPACE = 0x1,
			JOIN_IDENTICAL_VERTICES = 0x2,
			MAKE_LEFT_HANDED = 0x4,
			TRIANGULATE = 0x8,
			REMOVE_COMPONENT = 0x10,
			GEN_NORMALS = 0x20,
			GEN_SMOOTH_NORMALS = 0x40,
			SPLIT_LARGE_MESHES = 0x80,
			PRE_TRANSFORM_VERTICES = 0x100,
			LIMIT_BONE_WEIGHTS = 0x200,
			VALIDATE_DATA_STRUCTURE = 0x400,
			IMPROVE_CACHE_LOCALITY = 0x800,
			REMOVE_REDUNDANT_MATERIALS = 0x1000,
			FIX_INFACING_NORMALS = 0x2000,
			SORT_BY_PTYPE = 0x8000,
			FIND_DEGENERATES = 0x10000,
			FIND_INVALID_DATA = 0x20000,
			GEN_UV_COORDS = 0x40000,
			TRANSFORM_UV_COORDS = 0x80000,
			FIND_INSTANCES = 0x100000,
			OPTIMIZE_MESHES = 0x200000,
			OPTIMIZE_GRAPH = 0x400000,
			FLIP_UVS = 0x800000,
			FLIP_WINDING_ORDER = 0x1000000,
			SPLIT_BY_BONE_COUNT = 0x2000000,
			DEBONE = 0x4000000
		};

		inline ModelParserFlags operator~ (ModelParserFlags a) { return (ModelParserFlags)~(int)a; }
		inline ModelParserFlags operator| (ModelParserFlags a, ModelParserFlags b) { return (ModelParserFlags)((int)a | (int)b); }
		inline ModelParserFlags operator& (ModelParserFlags a, ModelParserFlags b) { return (ModelParserFlags)((int)a & (int)b); }
		inline ModelParserFlags operator^ (ModelParserFlags a, ModelParserFlags b) { return (ModelParserFlags)((int)a ^ (int)b); }
		inline ModelParserFlags& operator|= (ModelParserFlags& a, ModelParserFlags b) { return (ModelParserFlags&)((int&)a |= (int)b); }
		inline ModelParserFlags& operator&= (ModelParserFlags& a, ModelParserFlags b) { return (ModelParserFlags&)((int&)a &= (int)b); }
		inline ModelParserFlags& operator^= (ModelParserFlags& a, ModelParserFlags b) { return (ModelParserFlags&)((int&)a ^= (int)b); }
		
		class AssimpParser {
		public:
			bool LoadModel(const std::string& fileName, RESOURCES::ResourcePtr<RENDER::Model> model, ModelParserFlags parserFlags);
		private:
			const unsigned int MAX_BONE_WEIGHTS = 4;
			void processMaterials(const struct aiScene* scene, std::vector<std::string>& materials);;
			void processNode(void* transform, struct aiNode* node, const struct aiScene* scene, ResourcePtr<RENDER::Model> model);
			void processMesh(void* transform, struct aiMesh* mesh, const struct aiScene* scene, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
			void loadBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene, ResourcePtr<RENDER::Model> model);
			void setVertexBoneData(Vertex& vertex, int boneID, float weight);
		};
	}
}
