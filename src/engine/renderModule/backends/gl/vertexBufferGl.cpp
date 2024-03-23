#include "vertexBufferGl.h"
#ifdef OPENGL_BACKEND

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

VertexArray::VertexArray() {
#ifndef USING_GLES
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
#endif
}

VertexArray::~VertexArray() {
#ifndef USING_GLES
	glDeleteVertexArrays(1, &ID);
#endif
}

void VertexArray::bind() const {
#ifndef USING_GLES
	glBindVertexArray(ID);
#endif
}

void VertexArray::unbind() const {
#ifndef USING_GLES
	glBindVertexArray(0);
#endif
}

unsigned VertexArray::getID() const {
	return ID;
}

#endif


