#include "vertex.h"

using namespace IKIGAI;

Vertex::Vertex(const MATH::Vector3f& position, const MATH::Vector2f& texCoord, const MATH::Vector3f& normal,
	const MATH::Vector3f& tangent, const MATH::Vector3f& bitangent, std::array<int, 4> boneIDs, const MATH::Vector4f& weights)
{
	this->position = position;
	this->texCoord = texCoord;
	this->normal = normal;
	this->tangent = tangent;
	this->bitangent = bitangent;
	m_BoneIDs = std::move(boneIDs);
	m_Weights = weights;
}
