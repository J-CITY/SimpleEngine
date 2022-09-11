#include "indexBuffer.h"

KUMA::RENDER::IndexBuffer::IndexBuffer(std::span<unsigned> data) {
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
}

KUMA::RENDER::IndexBuffer::IndexBuffer(std::span<unsigned> data, UsageType type) {
	glGenBuffers(1, &m_bufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), UsageTypeToEnum[static_cast<int>(type)]);
}

KUMA::RENDER::IndexBuffer::~IndexBuffer() {
	glDeleteBuffers(1, &m_bufferID);
}

void KUMA::RENDER::IndexBuffer::bind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
}

void KUMA::RENDER::IndexBuffer::unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned KUMA::RENDER::IndexBuffer::getID() const {
	return m_bufferID;
}
