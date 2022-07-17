#pragma once

#include <cassert>
#include <cstdint>
import glmath;

namespace  Game 
{

	struct i8Vec3 {
		uint8_t x;
		uint8_t y;
		uint8_t z;

		i8Vec3 operator=(const KUMA::MATHGL::Vector3& vec) {
			//assert(floor(vec.x) <= CHUNK_SIZE_X);
			//assert(floor(vec.y) < 255);
			//assert(floor(vec.z) <= CHUNK_SIZE_Z);

			x = floor(vec.x);
			y = floor(vec.y);
			z = floor(vec.z);

			return *this;
		}
		i8Vec3 operator=(const KUMA::MATHGL::Vector4& vec) {
			//assert(floor(vec.x) <= CHUNK_SIZE_X);
			//assert(floor(vec.y) < 255);
			//assert(floor(vec.z) <= CHUNK_SIZE_Z);

			x = floor(vec.x);
			y = floor(vec.y);
			z = floor(vec.z);

			return *this;
		}
	};

	struct i16Vec2 {
		uint16_t x;
		uint16_t y;
	};

	struct Vertex {
		i8Vec3 position;
		uint8_t lighting_level;
		i16Vec2 texCoord;
		uint8_t block_face_lighting;
	};
};
