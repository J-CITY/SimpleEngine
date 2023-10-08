#include "indexBufferGl.h"
#ifdef OPENGL_BACKEND
#include "vertexBufferGl.h"

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

IndexBufferGl::IndexBufferGl(std::span<unsigned> data) {
	sz = data.size();
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
}

IndexBufferGl::IndexBufferGl(std::span<unsigned> data, UsageType type) {
	sz = data.size();
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), UsageTypeToEnum[static_cast<int>(type)]);
}

IndexBufferGl::~IndexBufferGl() {
	glDeleteBuffers(1, &m_bufferID);
}

void IndexBufferGl::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
}

void IndexBufferGl::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned IndexBufferGl::getID() const {
	return m_bufferID;
}
#endif
