#pragma once
#ifdef DX12_BACKEND
#include "meshDx12.h"

#include <assimp/scene.h>
#include "../interface/meshInterface.h"
#include "../interface/modelInterface.h"
//#include "raytracing/rayTracedScene.h"
namespace IKIGAI::RENDER {
	class MaterialRT;

	class ModelDx12 : public ModelInterface {
		friend class RESOURCES::ModelLoader;
		friend class RESOURCES::AssimpParser;
	public:
		ModelDx12(const std::string& p_path);
		~ModelDx12();
	};
}

#endif
