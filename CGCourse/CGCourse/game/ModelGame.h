#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>

#include "Vertex.h"
#include "Biome.h"
#include "BlockDatabase.h"


namespace Game
{
	struct __ModelVertex
	{
		KUMA::MATHGL::Vector3 position;
		i16Vec2 tex_coords;
	};

	class ModelGame
	{
	public :
		ModelGame(BlockType type) 
		{
			const std::array<uint16_t, 8>& block_coords = BlockDatabase::GetBlockTexture(type, BlockDatabase::BlockFaceType::front);

			AddVertex(KUMA::MATHGL::Vector3(0, 0, 0), { block_coords[0], block_coords[1] });
			AddVertex(KUMA::MATHGL::Vector3(1, 0, 1), { block_coords[2], block_coords[3] });
			AddVertex(KUMA::MATHGL::Vector3(1, 1, 1), { block_coords[4], block_coords[5] });
			AddVertex(KUMA::MATHGL::Vector3(0, 1, 0), { block_coords[6], block_coords[7] });
			AddVertex(KUMA::MATHGL::Vector3(1, 0, 0), { block_coords[0], block_coords[1] });
			AddVertex(KUMA::MATHGL::Vector3(0, 0, 1), { block_coords[2], block_coords[3] });
			AddVertex(KUMA::MATHGL::Vector3(0, 1, 1), { block_coords[4], block_coords[5] });
			AddVertex(KUMA::MATHGL::Vector3(1, 1, 0), { block_coords[6], block_coords[7] });
		}

		std::vector<__ModelVertex> p_ModelVertices;

	protected :
		void AddVertex(const KUMA::MATHGL::Vector3& pos, const i16Vec2 tex_coords)
		{
			p_ModelVertices.push_back({ pos, tex_coords });
		}

	private :
	};
}