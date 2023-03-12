#pragma once
#ifdef VULKAN_BACKEND
#include "meshVk.h"

#include <assimp/scene.h>
#include "../interface/meshInterface.h"
#include "../interface/modelInterface.h"
//#include "raytracing/rayTracedScene.h"
namespace KUMA::RENDER {
	class MaterialRT;

	class ModelVk : public ModelInterface {
		friend class RESOURCES::ModelLoader;
		friend class RESOURCES::AssimpParser;

	public:
		[[nodiscard]] const std::vector<std::shared_ptr<MeshInterface>>& getMeshes() const;
		[[nodiscard]] const std::vector<std::string>& getMaterialNames() const;

		//private:
		ModelVk(const std::string& p_path);
		~ModelVk();

	public:


	public:
		void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
		std::unique_ptr<VertexBufferVk<Vertex>> mVertexBuffer;
		std::unique_ptr<IndexBufferVk> mIndexBuffer;
	};
}

#endif
