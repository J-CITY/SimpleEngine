#include "primitiveRender.h"

#include <vector>
#include "../resourceManager/shaderManager.h"
#include "../resourceManager/resource/mesh.h"

KUMA::RENDER::PrimitiveRender::PrimitiveRender(BaseRender& render): render(render) {
	std::vector<Vertex> vertices;
	vertices.push_back({
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector2f{0, 0},
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector3{0, 0, 0}
	});
	vertices.push_back({
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector2f{0, 0},
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector3{0, 0, 0},
		MATHGL::Vector3{0, 0, 0}
	});

	lineMesh = std::shared_ptr<RESOURCES::Mesh>(new RESOURCES::Mesh(vertices, {0, 1}, 0));
	lineShader = RESOURCES::ShaderLoader::CreateFromSource(vertexLineShader, fragmentLineShader);
	gridShader = RESOURCES::ShaderLoader::CreateFromSource(vertexGridShader, fragmentGridShader);
}

KUMA::RENDER::PrimitiveRender::~PrimitiveRender() = default;

void KUMA::RENDER::PrimitiveRender::setViewProjection(const MATHGL::Matrix4& p_viewProjection) {
	lineShader->bind();
	lineShader->setUniformMat4("viewProjection", p_viewProjection);
	lineShader->unbind();

	gridShader->bind();
	gridShader->setUniformMat4("viewProjection", p_viewProjection);
	gridShader->unbind();
}

void KUMA::RENDER::PrimitiveRender::drawLine(const MATHGL::Vector3& start, const MATHGL::Vector3& end,
	const MATHGL::Vector3& color, float lineWidth) {
	lineShader->bind();

	lineShader->setUniformVec3("start", start);
	lineShader->setUniformVec3("end", end);
	lineShader->setUniformVec3("color", color);

	render.setRasterizationMode(RasterizationMode::LINE);
	render.setRasterizationLinesWidth(lineWidth);
	render.draw(*lineMesh, PrimitiveMode::LINES);
	render.setRasterizationLinesWidth(1.0f);
	render.setRasterizationMode(RasterizationMode::FILL);

	lineShader->unbind();
}

void KUMA::RENDER::PrimitiveRender::drawGrid(const MATHGL::Vector3& viewPos, const MATHGL::Vector3& color,
	int32_t gridSize, float linear, float quadratic, float fadeThreshold, float p_lineWidth) {
	gridShader->bind();
	gridShader->setUniformVec3("color", color);
	gridShader->setUniformVec3("viewPos", viewPos);
	gridShader->setUniformFloat("linear", linear);
	gridShader->setUniformFloat("quadratic", quadratic);
	gridShader->setUniformFloat("fadeThreshold", fadeThreshold);

	render.setRasterizationMode(RasterizationMode::LINE);
	render.setRasterizationLinesWidth(p_lineWidth);
	render.setCapability(RenderingCapability::BLEND, true);

	for (int32_t i = -gridSize + 1; i < gridSize; ++i) {
		gridShader->setUniformVec3("start", {-(float)gridSize + std::floor(viewPos.x), 0.f, (float)i + std::floor(viewPos.z)});
		gridShader->setUniformVec3("end", {(float)gridSize + std::floor(viewPos.x), 0.f, (float)i + std::floor(viewPos.z)});
		render.draw(*lineMesh, PrimitiveMode::LINES);

		gridShader->setUniformVec3("start", {(float)i + std::floor(viewPos.x), 0.f, -(float)gridSize + std::floor(viewPos.z)});
		gridShader->setUniformVec3("end", {(float)i + std::floor(viewPos.x), 0.f, (float)gridSize + std::floor(viewPos.z)});
		render.draw(*lineMesh, PrimitiveMode::LINES);
	}

	render.setCapability(RenderingCapability::BLEND, false);
	render.setRasterizationLinesWidth(1.0f);
	render.setRasterizationMode(RasterizationMode::FILL);
	gridShader->unbind();
}
