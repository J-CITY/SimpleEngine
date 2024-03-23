#include "vertex.h"

using namespace IKIGAI;

Vertex::Vertex(const MATH::Vector3f& position, const MATH::Vector2f& texCoord,
	const  MATH::Vector3f& normal, const  MATH::Vector3f& tangent,
	const  MATH::Vector3f& bitangent) {
	this->position = position;
	this->texCoord = texCoord;
	this->normal = normal;
	this->tangent = tangent;
	this->bitangent = bitangent;
}
