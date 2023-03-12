#include "render.h"
/*#include "frustum.h"
#include <GL/glew.h>
#include "../resourceManager/resource/mesh.h"
#include "material.h"

using namespace KUMA;
using namespace KUMA::RENDER;

GLenum BlendTable [] = {
	0x00000000,
	GL_ZERO,
	GL_ONE,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA,
};

GLenum depthFuncTable [] = {
	GL_EQUAL,
	GL_NOTEQUAL,
	GL_LESS,
	GL_GREATER,
	GL_LEQUAL,
	GL_GEQUAL,
	GL_ALWAYS,
	GL_NEVER,
};

void BaseRender::useDepthFunction(DepthFunction function) {
	glDepthFunc(depthFuncTable[(size_t)function]);
}

void BaseRender::useCulling(bool value, bool counterClockWise, bool cullBack) {
	// culling 
	if (value) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	// point order
	if (counterClockWise) {
		glFrontFace(GL_CCW);
	}
	else {
		glFrontFace(GL_CW);
	}

	// back / front culling
	if (cullBack) {
		glCullFace(GL_BACK);
	}
	else {
		glCullFace(GL_FRONT);
	}
}

void BaseRender::drawIndices(PrimitiveMode primitive, size_t indexCount, size_t indexOffset) {
	glDrawElements(
		static_cast<GLenum>(primitive),
		indexCount,
		GL_UNSIGNED_INT,
		(const void*)(indexOffset * sizeof(int))
	);
}

void BaseRender::drawIndicesBaseVertex(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex) {
	glDrawElementsBaseVertex(
		static_cast<GLenum>(primitive),
		indexCount,
		GL_UNSIGNED_INT,
		(void*)(indexOffset * sizeof(unsigned)),
		baseVertex
	);
}

void BaseRender::drawIndicesBaseVertexInstanced(PrimitiveMode primitive, size_t indexCount, size_t indexOffset,
                                                size_t baseVertex, size_t instanceCount, size_t baseInstance) {
	glDrawElementsInstancedBaseVertexBaseInstance(
		static_cast<GLenum>(primitive),
		indexCount,
		GL_UNSIGNED_INT,
		(void*)(indexOffset * sizeof(unsigned)),
		instanceCount,
		baseVertex,
		baseInstance
	);
}

void BaseRender::setPatchSize(int sz) const {
	glPatchParameteri(GL_PATCH_VERTICES, sz);
}

void BaseRender::useBlendFactors(BlendFactor src, BlendFactor dist) {
	if (src == BlendFactor::NONE || dist == BlendFactor::NONE) {
		glDisable(GL_BLEND);
	}
	else {
		glEnable(GL_BLEND);
		glBlendFunc(BlendTable[(size_t)src], BlendTable[(size_t)dist]);
	}
}

void BaseRender::useReversedDepth(bool value) {
	if (value) {
		glClearDepth(0.0f);
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		useDepthFunction(DepthFunction::GREATER_EQUAL);
	}
	else {
		glClearDepth(1.0f);
		glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		useDepthFunction(DepthFunction::LESS);
	}
}

//std::shared_ptr<KUMA::RESOURCES::Animator> BaseRender::animator;

BaseRender::BaseRender(GL_SYSTEM::GlManager& driver) : driver(driver), state(0) {
}

void BaseRender::setClearColor(float red, float green, float blue, float alpha) {
	glClearColor(red, green, blue, alpha);
}

void BaseRender::clear(bool colorBuffer, bool depthBuffer, bool stencilBuffer) {
	glClear(
		(colorBuffer ? GL_COLOR_BUFFER_BIT : 0) |
		(depthBuffer ? GL_DEPTH_BUFFER_BIT : 0) |
		(stencilBuffer ? GL_STENCIL_BUFFER_BIT : 0)
	);
}

void BaseRender::setRasterizationLinesWidth(float width) {
	glLineWidth(width);
}

void BaseRender::setRasterizationMode(RasterizationMode rasterizationMode) {
	glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(rasterizationMode));
}

void BaseRender::setCapability(RenderingCapability capability, bool value) {
	(value ? glEnable : glDisable)(static_cast<GLenum>(capability));
}

bool BaseRender::getCapability(RenderingCapability capability) const {
	return glIsEnabled(static_cast<GLenum>(capability));
}

void BaseRender::setStencilAlgorithm(ComparaisonAlgorithm algorithm, int32_t reference, uint32_t mask) {
	glStencilFunc(static_cast<GLenum>(algorithm), reference, mask);
}

void BaseRender::setDepthAlgorithm(ComparaisonAlgorithm algorithm) {
	glDepthFunc(static_cast<GLenum>(algorithm));
}

void BaseRender::setStencilMask(uint32_t mask) {
	glStencilMask(mask);
}

void BaseRender::setStencilOperations(Operation stencilFail, Operation depthFail, Operation bothPass) {
	glStencilOp(static_cast<GLenum>(stencilFail), static_cast<GLenum>(depthFail), static_cast<GLenum>(bothPass));
}

void BaseRender::setCullFace(CullFace cullFace) {
	glCullFace(static_cast<GLenum>(cullFace));
}

void BaseRender::setDepthWriting(bool enable) {
	glDepthMask(enable);
}

void BaseRender::setColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) {
	glColorMask(enableRed, enableGreen, enableBlue, enableAlpha);
}

void BaseRender::setColorWriting(bool enable) {
	glColorMask(enable, enable, enable, enable);
}

void BaseRender::setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
}

void BaseRender::readPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelDataFormat format, PixelDataType type, void* data) {
	glReadPixels(x, y, width, height, static_cast<GLenum>(format), static_cast<GLenum>(type), data);
}

bool BaseRender::getBool(GLenum parameter) {
	GLboolean result;
	glGetBooleanv(parameter, &result);
	return static_cast<bool>(result);
}

bool BaseRender::getBool(GLenum parameter, uint32_t index) {
	GLboolean result;
	glGetBooleani_v(parameter, index, &result);
	return static_cast<bool>(result);
}

int BaseRender::getInt(GLenum parameter) {
	GLint result;
	glGetIntegerv(parameter, &result);
	return static_cast<int>(result);
}

int BaseRender::getInt(GLenum parameter, uint32_t index) {
	GLint result;
	glGetIntegeri_v(parameter, index, &result);
	return static_cast<int>(result);
}

float BaseRender::getFloat(GLenum parameter) {
	GLfloat result;
	glGetFloatv(parameter, &result);
	return static_cast<float>(result);
}

float BaseRender::getFloat(GLenum parameter, uint32_t index) {
	GLfloat result;
	glGetFloati_v(parameter, index, &result);
	return static_cast<float>(result);
}

double BaseRender::getDouble(GLenum parameter) {
	GLdouble result;
	glGetDoublev(parameter, &result);
	return static_cast<double>(result);
}

double BaseRender::getDouble(GLenum parameter, uint32_t index) {
	GLdouble result;
	glGetDoublei_v(parameter, index, &result);
	return static_cast<double>(result);
}

int64_t BaseRender::getInt64(GLenum parameter) {
	GLint64 result;
	glGetInteger64v(parameter, &result);
	return static_cast<int64_t>(result);
}

int64_t BaseRender::getInt64(GLenum parameter, uint32_t index) {
	GLint64 result;
	glGetInteger64i_v(parameter, index, &result);
	return static_cast<int64_t>(result);
}

std::string BaseRender::getString(GLenum parameter) {
	const GLubyte* result = glGetString(parameter);
	return result ? reinterpret_cast<const char*>(result) : std::string();
}

std::string BaseRender::getString(GLenum parameter, uint32_t index) {
	const GLubyte* result = glGetStringi(parameter, index);
	return result ? reinterpret_cast<const char*>(result) : std::string();
}

void BaseRender::clearFrameInfo() {
	frameInfo.batchCount = 0;
	frameInfo.instanceCount = 0;
	frameInfo.polyCount = 0;
}

void BaseRender::draw(const RESOURCES::Mesh& mesh, PrimitiveMode primitiveMode, uint32_t instances) {
	if (instances > 0) {
		frameInfo.batchCount++;
		frameInfo.instanceCount += instances;
		frameInfo.polyCount += (mesh.getIndexCount() / 3) * instances;

		mesh.bind();
		
		if (mesh.getIndexCount() > 0) {
			// EBO
			if (instances == 1) {
				glDrawElements(static_cast<GLenum>(primitiveMode), mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr);
			}
			else {
				glDrawElementsInstanced(static_cast<GLenum>(primitiveMode), mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr, instances);
			}
		}
		else {
			// EBO
			if (instances == 1) {
				//glPatchParameteri(GL_PATCH_VERTICES, 4);
				//glDrawArrays(GL_PATCHES, 0, 4 * 20 * 20);
				glDrawArrays(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount());
			}
			else {
				glDrawArraysInstanced(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount(), instances);
			}
		}
		mesh.unbind();
	}
}

uint8_t BaseRender::fetchGLState() {
	
	uint8_t result = 0;

	GLboolean cMask[4];
	glGetBooleanv(GL_COLOR_WRITEMASK, cMask);

	if (getBool(GL_DEPTH_WRITEMASK))						result |= 0b0000'0001;
	if (cMask[0])											result |= 0b0000'0010;
	if (getCapability(RenderingCapability::BLEND))			result |= 0b0000'0100;
	if (getCapability(RenderingCapability::CULL_FACE))		result |= 0b0000'1000;
	if (getCapability(RenderingCapability::DEPTH_TEST))	result |= 0b0001'0000;

	switch (static_cast<CullFace>(getInt(GL_CULL_FACE))) {
	case CullFace::BACK:			result |= 0b0010'0000; break;
	case CullFace::FRONT:			result |= 0b0100'0000; break;
	case CullFace::FRONT_AND_BACK:  result |= 0b0110'0000; break;
	}

	return result;
}

void BaseRender::applyStateMask(uint8_t mask) {
	if (mask != state) {
		if ((mask & 0x01) != (state & 0x01))	setDepthWriting(mask & 0x01);
		if ((mask & 0x02) != (state & 0x02))	setColorWriting(mask & 0x02);
		if ((mask & 0x04) != (state & 0x04))	setCapability(RenderingCapability::BLEND, mask & 0x04);
		if ((mask & 0x08) != (state & 0x08))	setCapability(RenderingCapability::CULL_FACE, mask & 0x8);
		if ((mask & 0x10) != (state & 0x10))	setCapability(RenderingCapability::DEPTH_TEST, mask & 0x10);

		if ((mask & 0x08) && ((mask & 0x20) != (state & 0x20) || (mask & 0x40) != (state & 0x40))) {
			int backBit = mask & 0x20;
			int frontBit = mask & 0x40;
			setCullFace(backBit && frontBit ? CullFace::FRONT_AND_BACK : 
				(backBit ? CullFace::BACK : CullFace::FRONT));
		}
		state = mask;
	}
}

void BaseRender::setState(uint8_t st) {
	state = st;
}

const BaseRender::FrameInfo& BaseRender::getFrameInfo() const {
	return frameInfo;
}

void BaseRender::useDepthBufferMask(bool value) {
	glDepthMask(value);
}
*/