#pragma once
#include <iostream>
#include "Chunk.h"
#include "TextureAtlas.h"
#include "../engine/resourceManager/resource/shader.h"


namespace SE {
	namespace MATHGL {
		struct Vector4;
	}
}

namespace KUMA {
	namespace ECS {
		class CameraComponent;
	}
}

namespace Game {
	class Renderer {
	public : 

		Renderer();

		void StartChunkRendering(KUMA::ECS::CameraComponent* camera, const KUMA::MATHGL::Vector4& ambient_light, int render_distance, const KUMA::MATHGL::Vector4& sun_position);
		void RenderTransparentChunk(Chunk* chunk);
		void RenderChunk(Chunk* chunk);
		void EndChunkRendering();

		void StartChunkModelRendering(KUMA::ECS::CameraComponent* camera, const KUMA::MATHGL::Vector4& ambient_light, int render_distance, const KUMA::MATHGL::Vector4& sun_position);
		void RenderChunkModels(Chunk* chunk);
		void EndChunkModelRendering();

		std::shared_ptr<KUMA::RESOURCES::Texture> GetAtlasTexture() { return m_BlockAtlas; }

	private:
		KUMA::RENDER::VertexBuffer<float> m_VBO;
		KUMA::RENDER::VertexArray m_VAO;
		std::shared_ptr<KUMA::RESOURCES::Shader> m_DefaultChunkShader;
		std::shared_ptr<KUMA::RESOURCES::Shader> m_DefaultChunkModelShader;
		std::shared_ptr<KUMA::RESOURCES::Texture> m_BlockAtlas;
	};
}
