#pragma once

#include <iostream>
#include <queue>

import glmath;

namespace Game
{
	class Chunk;

	// LightNode structures for flood fill lighting 

	class LightNode
	{
	public :

		LightNode(const KUMA::MATHGL::Vector3& position, Chunk* chunk) : p_Position(position), p_Chunk(chunk)
		{

		}

		KUMA::MATHGL::Vector3 p_Position;
		Chunk* p_Chunk;
	};

	class LightRemovalNode
	{
	public : 
		LightRemovalNode(const KUMA::MATHGL::Vector3& position, int light, Chunk* chunk) : p_Position(position), p_LightValue(light), p_Chunk(chunk)
		{
			
		}

		KUMA::MATHGL::Vector3 p_Position;
		uint8_t p_LightValue;
		Chunk* p_Chunk;
	};
}
