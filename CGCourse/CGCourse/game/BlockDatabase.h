#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include "Block.h"

namespace Game {
	namespace BlockDatabase {
		enum BlockFaceType {
			top = 0,
			bottom,
			left,
			right,
			front,
			backward
		};
		
		const std::array<uint16_t, 8>& GetBlockTexture(BlockType block_type, BlockFaceType face_type);
		const std::string& GetBlockName(BlockType block_type);
		const std::string& GetBlockSoundPath(BlockType type);
	}
}
