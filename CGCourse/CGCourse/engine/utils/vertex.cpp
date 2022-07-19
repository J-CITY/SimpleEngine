#include "vertex.h"

using namespace KUMA;

Vertex::Vertex(const MATHGL::Vector3& position, const MATHGL::Vector2f& texCoord,
	const  MATHGL::Vector3& normal, const  MATHGL::Vector3& tangent,
	const  MATHGL::Vector3& bitangent) {
	this->position = position;
	this->texCoord = texCoord;
	this->normal = normal;
	this->tangent = tangent;
	this->bitangent = bitangent;
}
