#pragma once
#include <iostream>

constexpr int CHUNK_SIZE_X =16;
constexpr int CHUNK_SIZE_Y =255;
constexpr int CHUNK_SIZE_Z =16;
constexpr int MAX_STRUCTURE_X =10;
constexpr int MAX_STRUCTURE_Y =10;
constexpr int MAX_STRUCTURE_Z = 10;

namespace Game
{
	enum Biome : uint8_t
	{
		Grassland = 0,
		Plains,
		Ocean,
		Desert,
		Jungle
	};

	enum class ChunkMeshState {
		BUILT=0,
		UNBUILT,
		ERR
	};

	enum class ChunkLightMapState {
		MODIFIED_LIGHT_MAP = 0,
		UNMODIFIED_LIGHT_MAP
	};

	enum class ChunkState {
		UNGENERETED = 5,
		GENERETED,
		CHANGED,
		UNDEFINED
	};
}
