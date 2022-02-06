#include "Renderer.h"
#include "../engine/ecs/components/cameraComponent.h"
#include "../engine/resourceManager/shaderManager.h"

namespace Game
{
	Renderer::Renderer()
	{
		// Create and compile the shaders

		m_DefaultChunkShader = KUMA::RESOURCES::ShaderLoader().createResource("shaders\\BlockRendererVertex.glsl");
		m_DefaultChunkModelShader = KUMA::RESOURCES::ShaderLoader().createResource("shaders\\ModelRendererVertex.glsl");
		
		m_BlockAtlas = KUMA::RESOURCES::TextureLoader().createResource("textures\\64x64_sheet.png");
	}

	void Renderer::RenderChunk(Chunk* chunk)
	{
		if (chunk->GetChunkMesh()->p_VerticesCount > 0)
		{
			m_DefaultChunkShader->setUniformInt("u_ChunkX", chunk->p_Position.x);
			m_DefaultChunkShader->setUniformInt("u_ChunkZ", chunk->p_Position.z);

			chunk->GetChunkMesh()->StaticIBO.bind();
			chunk->GetChunkMesh()->p_VAO.bind();
			glDrawArrays(GL_TRIANGLES, 0, chunk->GetChunkMesh()->p_VerticesCount);
			//(glDrawElements(GL_TRIANGLES, floor(chunk->GetChunkMesh()->p_VerticesCount / 4) * 6, GL_UNSIGNED_INT, 0));
			//(glDrawElements(GL_TRIANGLES, chunk->GetChunkMesh()->isize, GL_UNSIGNED_INT, 0));

			chunk->GetChunkMesh()->p_VAO.unbind();
		}
	}

	void Renderer::RenderTransparentChunk(Chunk* chunk) {
		if (chunk->GetChunkMesh()->p_TransparentVerticesCount > 0) {
			m_DefaultChunkShader->setUniformInt("u_ChunkX", chunk->p_Position.x);
			m_DefaultChunkShader->setUniformInt("u_ChunkZ", chunk->p_Position.z);

			chunk->GetChunkMesh()->p_TransparentVAO.bind();
			//(glDrawElements(GL_TRIANGLES, floor(chunk->GetChunkMesh()->p_TransparentVerticesCount / 4) * 6, GL_UNSIGNED_INT, 0));
			glDrawArrays(GL_TRIANGLES, 0, chunk->GetChunkMesh()->p_TransparentVerticesCount);
			chunk->GetChunkMesh()->p_TransparentVAO.unbind();
		}
	}

	void Renderer::StartChunkRendering(KUMA::ECS::CameraComponent* camera, const KUMA::MATHGL::Vector4& ambient_light, int render_distance, const KUMA::MATHGL::Vector4& sun_position) {
		m_DefaultChunkShader->bind();
		m_BlockAtlas->bind(0);
		m_DefaultChunkShader->setUniformInt("u_Texture", 0);
		m_DefaultChunkShader->setUniformVec4("u_AmbientLight", ambient_light);
		m_DefaultChunkShader->setUniformMat4("u_ViewProjection", camera->getCamera().getProjectionMatrix() * camera->getCamera().getViewMatrix());
		m_DefaultChunkShader->setUniformMat4("u_ViewMatrix", camera->getCamera().getViewMatrix());
		m_DefaultChunkShader->setUniformInt("u_RenderDistance", render_distance);
		m_DefaultChunkShader->setUniformInt("u_CHUNK_SIZE_X", CHUNK_SIZE_X);
		m_DefaultChunkShader->setUniformInt("u_CHUNK_SIZE_Z", CHUNK_SIZE_Z);
		m_DefaultChunkShader->setUniformFloat("u_SunPositionY", sun_position.y);
		m_DefaultChunkShader->setUniformVec4("u_FogColor", KUMA::MATHGL::Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // WHITE FOG
	}

	void Renderer::EndChunkRendering() {
		glUseProgram(0);
	}

	void Renderer::StartChunkModelRendering(KUMA::ECS::CameraComponent* camera, const KUMA::MATHGL::Vector4& ambient_light, int render_distance, const KUMA::MATHGL::Vector4& sun_position) {
		m_DefaultChunkModelShader->bind();
		m_BlockAtlas->bind(0);
		m_DefaultChunkModelShader->setUniformInt("u_Texture", 0);
		m_DefaultChunkModelShader->setUniformVec4("u_AmbientLight", ambient_light);
		m_DefaultChunkModelShader->setUniformMat4("u_ViewProjection", camera->getCamera().getProjectionMatrix() * camera->getCamera().getViewMatrix());
		m_DefaultChunkModelShader->setUniformMat4("u_ViewMatrix", camera->getCamera().getViewMatrix());
		m_DefaultChunkModelShader->setUniformInt("u_RenderDistance", render_distance);
		m_DefaultChunkModelShader->setUniformFloat("u_SunPositionY", sun_position.y);
		m_DefaultChunkShader->setUniformInt("u_CHUNK_SIZE_X", CHUNK_SIZE_X);
		m_DefaultChunkShader->setUniformInt("u_CHUNK_SIZE_Z", CHUNK_SIZE_Z);
		m_DefaultChunkModelShader->setUniformVec4("u_FogColor", KUMA::MATHGL::Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // WHITE FOG
		m_DefaultChunkModelShader->setUniformFloat("u_Time", 0.1f);
	}

	void Renderer::RenderChunkModels(Chunk* chunk) {
		if (chunk->GetChunkMesh()->p_ModelVerticesCount > 0) {
			m_DefaultChunkShader->setUniformInt("u_ChunkX", chunk->p_Position.x);
			m_DefaultChunkShader->setUniformInt("u_ChunkZ", chunk->p_Position.z);
			chunk->GetChunkMesh()->p_ModelVAO.bind();
			//(glDrawElements(GL_TRIANGLES, floor(chunk->GetChunkMesh()->p_ModelVerticesCount / 4) * 6, GL_UNSIGNED_INT, 0));
			glDrawArrays(GL_TRIANGLES, 0, chunk->GetChunkMesh()->p_ModelVerticesCount);
			chunk->GetChunkMesh()->p_ModelVAO.unbind();
		}
	}

	void Renderer::EndChunkModelRendering() {
		glUseProgram(0);
	}
}
