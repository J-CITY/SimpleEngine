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
		[[nodiscard]] const std::vector<std::shared_ptr<MeshInterface>>& getMeshes() const;
		[[nodiscard]] const std::vector<std::string>& getMaterialNames() const;

		//private:
		ModelDx12(const std::string& p_path);
		~ModelDx12();

	public:


	public:
		//void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
		//std::unique_ptr<VertexBufferVk<Vertex>> mVertexBuffer;
		//std::unique_ptr<IndexBufferVk> mIndexBuffer;
	};
}

#endif
