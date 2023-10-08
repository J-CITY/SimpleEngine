#pragma once

#include <array>

import glmath;

namespace IKIGAI {
	constexpr int MAX_BONE_INFLUENCE = 4;
	class Vertex {
	public:
		MATHGL::Vector3 position;
		MATHGL::Vector2f texCoord;
		MATHGL::Vector3 normal;
		MATHGL::Vector3 tangent;
		MATHGL::Vector3 bitangent;

		//bone indexes which will influence this vertex
		std::array<int, 4> m_BoneIDs = {-1,-1,-1,-1};
		//weights from each bone
		std::array<float, 4> m_Weights = {0.0f,0.0f,0.0f,0.0f};

		Vertex() = default;
		
		Vertex(const MATHGL::Vector3& position, const MATHGL::Vector2f& texCoord = MATHGL::Vector2f(0, 0),
			const  MATHGL::Vector3& normal = MATHGL::Vector3(0, 0, 0), const  MATHGL::Vector3& tangent = MATHGL::Vector3(0, 0, 0),
			const  MATHGL::Vector3& bitangent = MATHGL::Vector3(0, 0, 0));
	};
}
