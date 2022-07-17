#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include "Biome.h"
#include "ChunkMesh.h"
#include "FrustumAABB.h"
#include "../engine/ecs/components/component.h"
import glmath;
#include "Block.h"

namespace Game {
	class Chunk {
	public : 

		Chunk(const KUMA::MATHGL::Vector3 chunk_position);
		~Chunk();

		void SetBlock(BlockType type, const KUMA::MATHGL::Vector3& position);

		int GetTorchLightAt(int x, int y, int z);
		void SetTorchLightAt(int x, int y, int z, int light_val);

		void Construct();
		ChunkMesh* GetChunkMesh();

		Block* GetBlock(int x, int y, int z);

		const KUMA::MATHGL::Vector3 p_Position;
		ChunkMeshState p_MeshState;
		ChunkState p_ChunkState = ChunkState::UNGENERETED;
		std::array<std::array<std::array<Block, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z> p_ChunkContents;
		std::array<std::array<std::array<uint8_t, CHUNK_SIZE_X>, CHUNK_SIZE_Y>, CHUNK_SIZE_Z> p_ChunkLightInformation;
		ChunkLightMapState p_LightMapState;
		FrustumAABB p_ChunkFrustumAABB;

		// Total : 512 bytes for the height and biome maps
		std::array<std::array<uint8_t, CHUNK_SIZE_X>, CHUNK_SIZE_Z> p_HeightMap;
		std::array<std::array<Biome, CHUNK_SIZE_X>, CHUNK_SIZE_Z> p_BiomeMap;

	private :
		ChunkMesh m_ChunkMesh;
	};
}
