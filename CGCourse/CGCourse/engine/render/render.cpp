#include "render.h"
#include "Frustum.h"
#include "Camera.h"
#include "Model.h"
#include <GL/glew.h>


#include "Material.h"
#include "../utils/time/time.h"
#include "../resourceManager/resource/bone.h"
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

void BaseRender::useBlendFactors(BlendFactor src, BlendFactor dist) {
	if (src == BlendFactor::NONE || dist == BlendFactor::NONE) {
		glDisable(GL_BLEND);
	}
	else {
		glEnable(GL_BLEND);
		glBlendFunc(BlendTable[(size_t)src], BlendTable[(size_t)dist]);
	}
}

std::shared_ptr<KUMA::RESOURCES::Animator> BaseRender::animator;

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
float __time = 0.0f;
void BaseRender::draw(RESOURCES::Mesh& mesh, Material& material, PrimitiveMode primitiveMode, uint32_t instances) {
	if (instances > 0) {
		frameInfo.batchCount++;
		frameInfo.instanceCount += instances;
		frameInfo.polyCount += (mesh.getIndexCount() / 3) * instances;

		mesh.bind();

		/*if (mesh.m_pScene) {
			std::vector<MATHGL::Matrix4> Transforms;
			__time += KUMA::TIME::Timer::instance()->getDeltaTime();
			mesh.boneTransform(__time / 10.0f, Transforms);
		
			material.getShader()->setUniformInt("useBone", Transforms.empty() ? 0 : 1);
			for (unsigned i = 0; i < Transforms.size(); i++) {
				auto s = "gBones[" + std::to_string(i) + "]";
				glUniformMatrix4fv(material.getShader()->getUniformLocation(s), 1, GL_TRUE, &Transforms[i].data[0]);
			}
		}*/
		

		if (animator) {
			auto transforms = animator->GetFinalBoneMatrices();
			for (int i = 0; i < transforms.size(); ++i) {
				MATHGL::Matrix4 m = MATHGL::Matrix4(
					transforms[i][0][0], transforms[i][0][1], transforms[i][0][2], transforms[i][0][3],
					transforms[i][1][0], transforms[i][1][1], transforms[i][1][2], transforms[i][1][3],
					transforms[i][2][0], transforms[i][2][1], transforms[i][2][2], transforms[i][2][3],
					transforms[i][3][0], transforms[i][3][1], transforms[i][3][2], transforms[i][3][3]
				);
				//material.getShader()->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", MATHGL::Matrix4::Transpose(m));
				glUniformMatrix4fv(glGetUniformLocation(material.getShader()->id, std::string("finalBonesMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &transforms[i][0][0]);
			}
		}
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
				glDrawArrays(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount());
			}
			else {
				glDrawArraysInstanced(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount(), instances);
			}
		}

		mesh.unbind();
	}
}


void BaseRender::draw(RESOURCES::Mesh& mesh, PrimitiveMode primitiveMode, uint32_t instances) {
	if (instances > 0) {
		frameInfo.batchCount++;
		frameInfo.instanceCount += instances;
		frameInfo.polyCount += (mesh.getIndexCount() / 3) * instances;

		mesh.bind();

		if (mesh.getIndexCount() > 0) {
			if (instances == 1) {
				glDrawElements(static_cast<GLenum>(primitiveMode), mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr);
			}
			else {
				glDrawElementsInstanced(static_cast<GLenum>(primitiveMode), mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr, instances);
			}
		}
		else {
			if (instances == 1) {
				glDrawArrays(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount());
			}
			else {
				glDrawArraysInstanced(static_cast<GLenum>(primitiveMode), 0, mesh.getVertexCount(), instances);
			}
		}

		mesh.unbind();
	}
}

std::vector<std::reference_wrapper<RESOURCES::Mesh>> BaseRender::getMeshesInFrustum(
	const Model& model,
	const BoundingSphere& modelBoundingSphere,
	const ECS::Transform& modelTransform,
	const Frustum& frustum,
	CullingOptions cullingOptions
) {
	const bool frustumPerModel = isFlagSet(CullingOptions::FRUSTUM_PER_MODEL, cullingOptions);

	if (!frustumPerModel || frustum.boundingSphereInFrustum(modelBoundingSphere, modelTransform)) {
		std::vector<std::reference_wrapper<RESOURCES::Mesh>> result;

		const bool frustumPerMesh = isFlagSet(CullingOptions::FRUSTUM_PER_MESH, cullingOptions);

		const auto& meshes = model.getMeshes();

		for (auto mesh : meshes) {
			if (meshes.size() == 1 || !frustumPerMesh || frustum.boundingSphereInFrustum(mesh->getBoundingSphere(), modelTransform)) {
				result.emplace_back(*mesh);
			}
		}
		return result;
	}
	return {};
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

void BaseRender::setState(uint8_t state) {
	state = state;
}

const BaseRender::FrameInfo& BaseRender::getFrameInfo() const {
	return frameInfo;
}