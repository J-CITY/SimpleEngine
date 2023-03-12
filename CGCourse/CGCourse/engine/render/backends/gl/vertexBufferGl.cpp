#include "vertexBufferGl.h"
#ifdef OPENGL_BACKEND

using namespace KUMA;
using namespace KUMA::RENDER;

VertexArray::VertexArray() {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &ID);
}

void VertexArray::bind() const {
	glBindVertexArray(ID);
}

void VertexArray::unbind() const {
	glBindVertexArray(0);
}

unsigned VertexArray::getID() const {
	return ID;
}

#endif


