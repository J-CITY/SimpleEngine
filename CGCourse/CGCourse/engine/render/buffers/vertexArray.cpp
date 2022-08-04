#include "vertexArray.h"

KUMA::RENDER::VertexArray::VertexArray() {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
}

KUMA::RENDER::VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &ID);
}

void KUMA::RENDER::VertexArray::bind() const {
	glBindVertexArray(ID);
}

void KUMA::RENDER::VertexArray::unbind() const {
	glBindVertexArray(0);
}

unsigned KUMA::RENDER::VertexArray::getID() const {
	return ID;
}
