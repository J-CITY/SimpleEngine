#pragma once

#include <iostream>

#include "Noise/FastNoise.h"

#include "World.h"
#include "WorldGeneratorType.h"
#include "Chunk.h"
#include "Biome.h"
#include "Block.h"

namespace Game
{
	enum WorldStructureType {
		Trees,
		Cacti,
		Undefined
	};

	struct StructureBlock {
		Block block;
		int x, y, z;
	};

	class WorldStructure {
	public:

		WorldStructure() {
			p_StructureType = WorldStructureType::Undefined;
		}

		~WorldStructure() {
			p_Structure.clear();
		};

		WorldStructureType p_StructureType;
		std::vector<StructureBlock> p_Structure;

	protected:

		void SetBlock(int x, int y, int z, BlockType type) noexcept {
			StructureBlock b;

			b.x = x;
			b.y = y;
			b.z = z;
			b.block = {type};
			p_Structure.push_back(b);
		}

		void SetBlocksX(const KUMA::MATHGL::Vector3& position, int breadth, BlockType type) noexcept {
			for (int x = position.x; x < position.x + breadth; x++) {
				SetBlock(x, position.y, position.z, type);
			}
		}

		void SetBlocksZ(const KUMA::MATHGL::Vector3& position, int depth, BlockType type) noexcept {
			for (int z = position.z; z < position.z + depth; z++) {
				SetBlock(position.x, position.y, z, type);
			}
		}

		void SetBlocksY(const KUMA::MATHGL::Vector3& position, int height, BlockType type) noexcept {
			for (int y = position.y; y < position.y + height; y++) {
				SetBlock(position.x, y, position.z, type);
			}
		}

		void SetBlocksHorizontal(const KUMA::MATHGL::Vector3& position, int breadth, int depth, BlockType type) noexcept {
			for (int i = position.x; i < position.x + breadth; i++) {
				for (int j = position.z; j < position.z + depth; j++) {
					SetBlock(i, position.y, j, type);
				}
			}
		}
	};


	class TreeStructure : public WorldStructure {
	public:

		TreeStructure() {
			// Define the tree structure

			// Leaves
			SetBlocksHorizontal(KUMA::MATHGL::Vector3(0, 5, 0), 5, 5, BlockType::OakLeaves);

			SetBlock(0, 6, 0, BlockType::OakLeaves);
			SetBlock(0, 6, 4, BlockType::OakLeaves);
			SetBlock(4, 6, 0, BlockType::OakLeaves);
			SetBlock(4, 6, 4, BlockType::OakLeaves);
			SetBlocksHorizontal(KUMA::MATHGL::Vector3(1, 6, 1), 3, 3, BlockType::OakLeaves);
			SetBlock(2, 7, 2, BlockType::OakLeaves);
			SetBlock(2 + 1, 7, 2, BlockType::OakLeaves);
			SetBlock(2 - 1, 7, 2, BlockType::OakLeaves);
			SetBlock(2, 7, 2 + 1, BlockType::OakLeaves);
			SetBlock(2, 7, 2 - 1, BlockType::OakLeaves);
			SetBlock(2, 8, 2, BlockType::OakLeaves);

			// Bark
			SetBlocksY(KUMA::MATHGL::Vector3(2, 0, 2), 8, BlockType::OakLog);
		}
	};

	class CactusStructure : public WorldStructure {
	public:
		CactusStructure() {
			SetBlocksY(KUMA::MATHGL::Vector3(0, 0, 0), 6, BlockType::Cactus);
		}
	};

	
	Block* GetWorldBlock(const KUMA::MATHGL::Vector3& block_pos);

	void GenerateChunk(Chunk* chunk, const int WorldSeed, WorldGenerationType gen_type);
	void GenerateChunkFlora(Chunk* chunk, const int WorldSeed, WorldGenerationType gen_type);
}