#pragma once

#include <iostream>
#include <vector>

#include "Biome.h"
#include "BlockDatabase.h"
#include "Vertex.h"
#include "../engine/render/buffers/vertexArray.h"
#include "../engine/utils/vertex.h"
import glmath;
#include "Block.h"
#include "../engine/render/buffers/indexBuffer.h"

namespace Game {
	class Chunk;
	typedef std::array<std::array<std::array<Block, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z>* ChunkDataTypePtr;
	typedef std::array<std::array<std::array<uint8_t, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z>* ChunkLightDataTypePtr;

	// Forward declarations
	ChunkDataTypePtr _GetChunkDataForMeshing(int cx, int cz);
	ChunkLightDataTypePtr _GetChunkLightDataForMeshing(int cx, int cz);

	class ChunkMesh {
	public : 
		ChunkMesh();
		~ChunkMesh();

		bool ConstructMesh(Chunk* chunk, const KUMA::MATHGL::Vector3& chunk_pos);
		
		std::uint32_t p_VerticesCount;
		std::uint32_t p_TransparentVerticesCount;
		std::uint32_t p_ModelVerticesCount;
		KUMA::RENDER::VertexArray p_VAO;
		KUMA::RENDER::VertexArray p_TransparentVAO;
		KUMA::RENDER::VertexArray p_ModelVAO;

		int isize = 0;
		static KUMA::RENDER::IndexBuffer StaticIBO;
	private : 

		void AddFace(Chunk* chunk, BlockDatabase::BlockFaceType face_type, 
			const KUMA::MATHGL::Vector3& position, BlockType type, uint8_t light_level, bool buffer = true);

		void AddModel(Chunk* chunk, const KUMA::MATHGL::Vector3& local_pos, BlockType type, float light_level);

		std::vector<Game::Vertex> m_Vertices;
		std::vector<Game::Vertex> m_TransparentVertices;
		std::vector<Game::Vertex> m_ModelVertices;
		KUMA::MATHGL::Vector4 m_TopFace[4];
		KUMA::MATHGL::Vector4 m_BottomFace[4];
		KUMA::MATHGL::Vector4 m_ForwardFace[4];
		KUMA::MATHGL::Vector4 m_BackFace[4];
		KUMA::MATHGL::Vector4 m_LeftFace[4];
		KUMA::MATHGL::Vector4 m_RightFace[4];

		KUMA::RENDER::VertexBuffer<Vertex> m_VBO;
		KUMA::RENDER::VertexBuffer<Vertex> m_TransparentVBO; // Vertex buffer for trasparent blocks
		KUMA::RENDER::VertexBuffer<Vertex> m_ModelVBO; // Vertex buffer for trasparent blocks
	};
}
