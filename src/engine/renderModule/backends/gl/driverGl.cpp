#include "driverGl.h"

#ifdef OPENGL_BACKEND
#include <gl/glew.h>
#include <string>
#include "../interface/meshInterface.h"

import logger;

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

GLenum PrimitiveModeTable[] = {
	GL_POINTS,
	GL_LINES,
	GL_LINE_LOOP,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_LINES_ADJACENCY,
	GL_LINE_STRIP_ADJACENCY,
	GL_TRIANGLES_ADJACENCY,
	GL_TRIANGLE_STRIP_ADJACENCY,
	GL_PATCHES
};
GLenum RenderingCapabilityTable[] = {
	GL_BLEND,
	GL_CULL_FACE,
	GL_DEPTH_TEST,
	GL_DITHER,
	GL_POLYGON_OFFSET_FILL,
	GL_SAMPLE_ALPHA_TO_COVERAGE,
	GL_SAMPLE_COVERAGE,
	GL_SCISSOR_TEST,
	GL_STENCIL_TEST,
	GL_MULTISAMPLE
};
GLenum RasterizationModeTable[] = {
	GL_POINT,
	GL_LINE,
	GL_FILL
};
GLenum ComparaisonAlgorithmTable[] = {
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

GLenum OperationTable[] = {
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_INCR_WRAP,
	GL_DECR,
	GL_DECR_WRAP,
	GL_INVERT
};
GLenum CullFaceTable[] = {
	GL_FRONT,
	GL_BACK,
	GL_FRONT_AND_BACK
};
GLenum PixelDataTypeTable[] = {
	GL_BYTE,
	GL_UNSIGNED_BYTE,
	GL_BITMAP,
	GL_SHORT,
	GL_UNSIGNED_SHORT,
	GL_INT,
	GL_UNSIGNED_INT,
	GL_FLOAT,
	GL_UNSIGNED_BYTE_3_3_2,
	GL_UNSIGNED_BYTE_2_3_3_REV,
	GL_UNSIGNED_SHORT_5_6_5,
	GL_UNSIGNED_SHORT_5_6_5_REV,
	GL_UNSIGNED_SHORT_4_4_4_4,
	GL_UNSIGNED_SHORT_4_4_4_4_REV,
	GL_UNSIGNED_SHORT_5_5_5_1,
	GL_UNSIGNED_SHORT_1_5_5_5_REV,
	GL_UNSIGNED_INT_8_8_8_8,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	GL_UNSIGNED_INT_10_10_10_2,
	GL_UNSIGNED_INT_2_10_10_10_REV
};
GLenum PixelDataFormatTable[] = {
	GL_COLOR_INDEX,
	GL_STENCIL_INDEX,
	GL_DEPTH_COMPONENT,
	GL_RED,
	GL_GREEN,
	GL_BLUE,
	GL_ALPHA,
	GL_RGB,
	GL_BGR,
	GL_RGBA,
	GL_BGRA,
	GL_LUMINANCE,
	GL_LUMINANCE_ALPHA
};

GLenum DepthFunctionTable[] = {
	GL_EQUAL,
	GL_NOTEQUAL,
	GL_LESS,
	GL_GREATER,
	GL_LEQUAL,
	GL_GEQUAL,
	GL_ALWAYS,
	GL_NEVER,
};

GLenum BlendTable[] = {
	0x0,
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


void DriverGl::setViewport(const ShaderInterface& shader, float x, float y, float w, float h) {
    glViewport(x, y, w, h);
}

void DriverGl::setScissor(const ShaderInterface& shader, int x, int y, unsigned w, unsigned h) {
    glScissor(x, y, w, h);
}

void DriverGl::drawIndexed(std::shared_ptr<ShaderInterface> shader, size_t indexCount) {
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void DriverGl::initGlew() {
	glewExperimental = GL_TRUE;
	const GLenum error = glewInit();
	if (error != GLEW_OK) {
		std::string message = "GlManager::ERROR init GLEW: ";
		std::string glewError = reinterpret_cast<const char*>(glewGetErrorString(error));
		LOG_INFO(message + glewError);
	}
}

int DriverGl::init() {
	initGlew();

#if defined(BEDUG) || defined(_DEBUG)
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	return 0;
}


void DriverGl::GLDebugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char* message, const void* userParam) {
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
		return;
	}

	std::string output;

	output += "OpenGL Debug Message:\n";
	output += "Debug message (" + std::to_string(id) + "): " + message + "\n";

	switch (source) {
	case GL_DEBUG_SOURCE_API:				output += "Source: API";				break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		output += "Source: Window System";		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	output += "Source: Shader Compiler";	break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		output += "Source: Third Party";		break;
	case GL_DEBUG_SOURCE_APPLICATION:		output += "Source: Application";		break;
	case GL_DEBUG_SOURCE_OTHER:				output += "Source: Other";				break;
	}

	output += "\n";

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:               output += "Type: Error";				break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: output += "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  output += "Type: Undefined Behaviour";	break;
	case GL_DEBUG_TYPE_PORTABILITY:         output += "Type: Portability";			break;
	case GL_DEBUG_TYPE_PERFORMANCE:         output += "Type: Performance";			break;
	case GL_DEBUG_TYPE_MARKER:              output += "Type: Marker";				break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          output += "Type: Push Group";			break;
	case GL_DEBUG_TYPE_POP_GROUP:           output += "Type: Pop Group";			break;
	case GL_DEBUG_TYPE_OTHER:               output += "Type: Other";				break;
	}

	output += "\n";

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:			output += "Severity: High";				break;
	case GL_DEBUG_SEVERITY_MEDIUM:			output += "Severity: Medium";			break;
	case GL_DEBUG_SEVERITY_LOW:				output += "Severity: Low";				break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	output += "Severity: Notification";		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:			LOG_ERROR(output);	break;
	case GL_DEBUG_SEVERITY_MEDIUM:			LOG_WARNING(output);	break;
	case GL_DEBUG_SEVERITY_LOW:				LOG_INFO(output);		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	LOG_INFO(output);			break;
	}
}

///

void DriverGl::useDepthFunction(DepthFunction function) {
	glDepthFunc(DepthFunctionTable[static_cast<size_t>(function)]);
}

void DriverGl::useCulling(bool value, bool counterClockWise, bool cullBack) {
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

void DriverGl::drawIndices(PrimitiveMode primitive, size_t indexCount, size_t indexOffset) {
	glDrawElements(
		PrimitiveModeTable[static_cast<GLenum>(primitive)],
		indexCount,
		GL_UNSIGNED_INT,
		(const void*)(indexOffset * sizeof(int))
	);
}

void DriverGl::drawIndicesBaseVertex(PrimitiveMode primitive, size_t indexCount, size_t indexOffset, size_t baseVertex) {
	glDrawElementsBaseVertex(
		PrimitiveModeTable[static_cast<GLenum>(primitive)],
		indexCount,
		GL_UNSIGNED_INT,
		(void*)(indexOffset * sizeof(unsigned)),
		baseVertex
	);
}

void DriverGl::drawIndicesBaseVertexInstanced(PrimitiveMode primitive, size_t indexCount, size_t indexOffset,
	size_t baseVertex, size_t instanceCount, size_t baseInstance) {
	glDrawElementsInstancedBaseVertexBaseInstance(
		PrimitiveModeTable[static_cast<GLenum>(primitive)],
		indexCount,
		GL_UNSIGNED_INT,
		(void*)(indexOffset * sizeof(unsigned)),
		instanceCount,
		baseVertex,
		baseInstance
	);
}

void DriverGl::setPatchSize(int sz) const {
	glPatchParameteri(GL_PATCH_VERTICES, sz);
}

void DriverGl::useBlendFactors(BlendFactor src, BlendFactor dist) {
	if (src == BlendFactor::NONE || dist == BlendFactor::NONE) {
		glDisable(GL_BLEND);
	}
	else {
		glEnable(GL_BLEND);
		glBlendFunc(BlendTable[static_cast<size_t>(src)], BlendTable[static_cast<size_t>(dist)]);
	}
}

void DriverGl::useReversedDepth(bool value) {
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

void DriverGl::setClearColor(float red, float green, float blue, float alpha) {
	glClearColor(red, green, blue, alpha);
}

void DriverGl::clear(bool colorBuffer, bool depthBuffer, bool stencilBuffer) {
	glClear(
		(colorBuffer ? GL_COLOR_BUFFER_BIT : 0) |
		(depthBuffer ? GL_DEPTH_BUFFER_BIT : 0) |
		(stencilBuffer ? GL_STENCIL_BUFFER_BIT : 0)
	);
}

void DriverGl::setRasterizationLinesWidth(float width) {
	glLineWidth(width);
}

void DriverGl::setRasterizationMode(RasterizationMode rasterizationMode) {
	glPolygonMode(GL_FRONT_AND_BACK, RasterizationModeTable[static_cast<GLenum>(rasterizationMode)]);
}

void DriverGl::setCapability(RenderingCapability capability, bool value) {
	(value ? glEnable : glDisable)(RenderingCapabilityTable[static_cast<GLenum>(capability)]);
}

bool DriverGl::getCapability(RenderingCapability capability) const {
	return glIsEnabled(RenderingCapabilityTable[static_cast<GLenum>(capability)]);
}

void DriverGl::setStencilAlgorithm(ComparaisonAlgorithm algorithm, int32_t reference, uint32_t mask) {
	glStencilFunc(static_cast<GLenum>(algorithm), reference, mask);
}

void DriverGl::setDepthAlgorithm(ComparaisonAlgorithm algorithm) {
	glDepthFunc(ComparaisonAlgorithmTable[static_cast<GLenum>(algorithm)]);
}

void DriverGl::setStencilMask(uint32_t mask) {
	glStencilMask(mask);
}

void DriverGl::setStencilOperations(Operation stencilFail, Operation depthFail, Operation bothPass) {
	glStencilOp(OperationTable[static_cast<GLenum>(stencilFail)], 
		OperationTable[static_cast<GLenum>(depthFail)], 
		OperationTable[static_cast<GLenum>(bothPass)]);
}

void DriverGl::setCullFace(CullFace cullFace) {
	glCullFace(CullFaceTable[static_cast<GLenum>(cullFace)]);
}

void DriverGl::setDepthWriting(bool enable) {
	glDepthMask(enable);
}

void DriverGl::setColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha) {
	glColorMask(enableRed, enableGreen, enableBlue, enableAlpha);
}

void DriverGl::setColorWriting(bool enable) {
	glColorMask(enable, enable, enable, enable);
}

void DriverGl::setViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
}

void DriverGl::readPixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, PixelDataFormat format, PixelDataType type, void* data) {
	glReadPixels(x, y, width, height, PixelDataFormatTable[static_cast<GLenum>(format)], PixelDataTypeTable[static_cast<GLenum>(type)], data);
}

bool DriverGl::getBool(GLenum parameter) {
	GLboolean result;
	glGetBooleanv(parameter, &result);
	return static_cast<bool>(result);
}

bool DriverGl::getBool(GLenum parameter, uint32_t index) {
	GLboolean result;
	glGetBooleani_v(parameter, index, &result);
	return static_cast<bool>(result);
}

int DriverGl::getInt(GLenum parameter) {
	GLint result;
	glGetIntegerv(parameter, &result);
	return static_cast<int>(result);
}

int DriverGl::getInt(GLenum parameter, uint32_t index) {
	GLint result;
	glGetIntegeri_v(parameter, index, &result);
	return static_cast<int>(result);
}

float DriverGl::getFloat(GLenum parameter) {
	GLfloat result;
	glGetFloatv(parameter, &result);
	return static_cast<float>(result);
}

float DriverGl::getFloat(GLenum parameter, uint32_t index) {
	GLfloat result;
	glGetFloati_v(parameter, index, &result);
	return static_cast<float>(result);
}

double DriverGl::getDouble(GLenum parameter) {
	GLdouble result;
	glGetDoublev(parameter, &result);
	return static_cast<double>(result);
}

double DriverGl::getDouble(GLenum parameter, uint32_t index) {
	GLdouble result;
	glGetDoublei_v(parameter, index, &result);
	return static_cast<double>(result);
}

int64_t DriverGl::getInt64(GLenum parameter) {
	GLint64 result;
	glGetInteger64v(parameter, &result);
	return static_cast<int64_t>(result);
}

int64_t DriverGl::getInt64(GLenum parameter, uint32_t index) {
	GLint64 result;
	glGetInteger64i_v(parameter, index, &result);
	return static_cast<int64_t>(result);
}

std::string DriverGl::getString(GLenum parameter) {
	const GLubyte* result = glGetString(parameter);
	return result ? reinterpret_cast<const char*>(result) : std::string();
}

std::string DriverGl::getString(GLenum parameter, uint32_t index) {
	const GLubyte* result = glGetStringi(parameter, index);
	return result ? reinterpret_cast<const char*>(result) : std::string();
}

void DriverGl::clearFrameInfo() {
	mFrameInfo.mBatchCount = 0;
	mFrameInfo.mInstanceCount = 0;
	mFrameInfo.mPolyCount = 0;
}

void DriverGl::draw(const MeshInterface& mesh, PrimitiveMode primitive, uint32_t instances) {
	if (instances > 0) {
		mFrameInfo.mBatchCount++;
		mFrameInfo.mInstanceCount += instances;
		mFrameInfo.mPolyCount += (mesh.getIndexCount() / 3) * instances;

		mesh.bind();
		if (mesh.getIndexCount() > 0) {
			// EBO
			if (instances == 1) {
				glDrawElements(PrimitiveModeTable[static_cast<GLenum>(primitive)], mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr);
			}
			else {
				glDrawElementsInstanced(PrimitiveModeTable[static_cast<GLenum>(primitive)], mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr, instances);
			}
		}
		else {
			// EBO
			if (instances == 1) {
				//glPatchParameteri(GL_PATCH_VERTICES, 4);
				//glDrawArrays(GL_PATCHES, 0, 4 * 20 * 20);
				glDrawArrays(PrimitiveModeTable[static_cast<GLenum>(primitive)], 0, mesh.getVertexCount());
			}
			else {
				glDrawArraysInstanced(PrimitiveModeTable[static_cast<GLenum>(primitive)], 0, mesh.getVertexCount(), instances);
			}
		}
		mesh.unbind();
	}
}

uint8_t DriverGl::fetchGLState() {

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

void DriverGl::applyStateMask(uint8_t mask) {
	if (mask != mState) {
		if ((mask & 0x01) != (mState & 0x01))	setDepthWriting(mask & 0x01);
		if ((mask & 0x02) != (mState & 0x02))	setColorWriting(mask & 0x02);
		if ((mask & 0x04) != (mState & 0x04))	setCapability(RenderingCapability::BLEND, mask & 0x04);
		if ((mask & 0x08) != (mState & 0x08))	setCapability(RenderingCapability::CULL_FACE, mask & 0x8);
		if ((mask & 0x10) != (mState & 0x10))	setCapability(RenderingCapability::DEPTH_TEST, mask & 0x10);

		if ((mask & 0x08) && ((mask & 0x20) != (mState & 0x20) || (mask & 0x40) != (mState & 0x40))) {
			int backBit = mask & 0x20;
			int frontBit = mask & 0x40;
			setCullFace(backBit && frontBit ? CullFace::FRONT_AND_BACK :
				(backBit ? CullFace::BACK : CullFace::FRONT));
		}
		mState = mask;
	}
}

void DriverGl::setState(uint8_t st) {
	mState = st;
}

const DriverGl::FrameInfo& DriverGl::getFrameInfo() const {
	return mFrameInfo;
}

void DriverGl::useDepthBufferMask(bool value) {
	glDepthMask(value);
}


#endif
