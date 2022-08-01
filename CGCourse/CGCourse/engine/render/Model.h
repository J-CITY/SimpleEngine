#pragma once
#include <vector>
#include "../resourceManager/serializerInterface.h"
#include "../resourceManager/resource/mesh.h"
#include "objects/BoundingSphere.h"

namespace KUMA {
	namespace RESOURCES {
		class ModelLoader;
		class AssimpParser;
	}
	
	namespace RENDER {
		struct BoneInfo {
			/*id is index in finalBoneMatrices*/
			int id;
			/*offset matrix transforms vertex from model space to bone space*/
			//MATHGL::Matrix4 offset;
			MATHGL::Matrix4 offset;
		};

		class Model{
			friend class RESOURCES::ModelLoader;
			friend class RESOURCES::AssimpParser;

		public:
			[[nodiscard]] const std::vector<RESOURCES::Mesh*>& getMeshes() const;
			[[nodiscard]] const std::vector<std::string>& getMaterialNames() const;
			[[nodiscard]] const BoundingSphere& getBoundingSphere() const;

		//private:
			Model(const std::string& p_path);
			~Model();

			void computeBoundingSphere();

		public:
			std::string path;

			auto& GetBoneInfoMap() { return m_BoneInfoMap; }
			int& GetBoneCount() { return m_BoneCounter; }
		public:
			std::vector<RESOURCES::Mesh*> meshes;
			std::vector<std::string> materialNames;

			BoundingSphere boundingSphere;
			std::map<std::string, BoneInfo> m_BoneInfoMap;
			int m_BoneCounter = 0;
			const int MAX_BONE_WEIGHTS = 4;
		};
	}
}
