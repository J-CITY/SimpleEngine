#pragma once

#include <array>

#include "mathModule/math.h"


namespace IKIGAI {
	constexpr int MAX_BONE_INFLUENCE = 4;
	class Vertex {
	public:
		MATH::Vector3f position;
		MATH::Vector2f texCoord;
		MATH::Vector3f normal;
		MATH::Vector3f tangent;
		MATH::Vector3f bitangent;

		//bone indexes which will influence this vertex
		std::array<int, 4> m_BoneIDs = {-1,-1,-1,-1};
		//weights from each bone
		std::array<float, 4> m_Weights = {0.0f,0.0f,0.0f,0.0f};

		Vertex() = default;
		
		Vertex(const MATH::Vector3f& position, const MATH::Vector2f& texCoord = MATH::Vector2f(0, 0),
			const  MATH::Vector3f& normal = MATH::Vector3f(0, 0, 0), const  MATH::Vector3f& tangent = MATH::Vector3f(0, 0, 0),
			const  MATH::Vector3f& bitangent = MATH::Vector3f(0, 0, 0));
	};
}
