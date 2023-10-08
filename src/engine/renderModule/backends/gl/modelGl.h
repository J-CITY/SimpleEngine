#pragma once
#ifdef OPENGL_BACKEND

import glmath;
#include "indexBufferGl.h"
#include "vertexBufferGl.h"
#include <memory>
#include <string>
#include <vector>

#include "../interface/meshInterface.h"
#include "../interface/modelInterface.h"

namespace IKIGAI
{
	class Vertex;
}

namespace IKIGAI {

	namespace RENDER {

		class ModelGl: public ModelInterface {
			friend class RESOURCES::ModelLoader;
			friend class RESOURCES::AssimpParser;

		public:
			[[nodiscard]] const std::vector<std::shared_ptr<MeshInterface>>& getMeshes() const;
			[[nodiscard]] const std::vector<std::string>& getMaterialNames() const;
			
			//private:
			ModelGl(const std::string& p_path);
			~ModelGl();

		public:


		public:
			void createBuffers(std::span<Vertex> p_vertices, std::span<uint32_t> p_indices);
			std::unique_ptr<VertexArray> vertexArray;
			std::unique_ptr<VertexBufferGl<Vertex>> vertexBuffer;
			std::unique_ptr<IndexBufferGl> indexBuffer;
		};
	}
}
#endif

