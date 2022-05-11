#pragma once

#include <string>
#include <vector>
#include <cmath>

#include "render.h"
#include "../utils/vertex.h"
#include "../resourceManager/shaderManager.h"
namespace KUMA {
	namespace RENDER {
		class PrimitiveRender {
		public:
			PrimitiveRender(BaseRender& render): render(render) {
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

			~PrimitiveRender() {
				RESOURCES::ShaderLoader::Destroy(lineShader);
				RESOURCES::ShaderLoader::Destroy(gridShader);
			}

			void setViewProjection(const MATHGL::Matrix4& p_viewProjection) {
				lineShader->bind();
				lineShader->setUniformMat4("viewProjection", p_viewProjection);
				lineShader->unbind();

				gridShader->bind();
				gridShader->setUniformMat4("viewProjection", p_viewProjection);
				gridShader->unbind();
			}

			void drawLine(const MATHGL::Vector3& start, const MATHGL::Vector3& end, const MATHGL::Vector3& color, float lineWidth = 1.0f) {
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

			
			void drawGrid(const MATHGL::Vector3& viewPos, const MATHGL::Vector3& color, int32_t gridSize = 50, float linear = 0.0f, float quadratic = 0.0f, float fadeThreshold = 0.0f, float p_lineWidth = 1.0f) {
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

		private:
			std::shared_ptr<RESOURCES::Shader> lineShader = nullptr;
			std::shared_ptr<RESOURCES::Shader> gridShader = nullptr;
			std::shared_ptr<RESOURCES::Mesh> lineMesh = nullptr;
			BaseRender& render;

			inline const static std::string vertexLineShader = R"(
			#version 460 core
			uniform vec3 start;
			uniform vec3 end;
			uniform mat4 viewProjection;
			void main(){
				vec3 position = gl_VertexID == 0 ? start : end;
			    gl_Position = viewProjection * vec4(position, 1.0);
			})";

			inline const static std::string fragmentLineShader = R"(
			#version 460 core
			uniform vec3 color;
			out vec4 FRAGMENT_COLOR;
			void main() {
				FRAGMENT_COLOR = vec4(color, 1.0);
			})";


			inline const static std::string vertexGridShader = R"(
			#version 460 core
			uniform vec3 start;
			uniform vec3 end;
			uniform mat4 viewProjection;
			out vec3 fragPos;
			void main(){
				vec3 position = gl_VertexID == 0 ? start : end;
				fragPos = position;
			    gl_Position = viewProjection * vec4(position, 1.0);
			})";

			inline const static std::string fragmentGridShader = R"(
			#version 460 core
			uniform vec3 color;
			uniform vec3 viewPos;
			uniform float linear;
			uniform float quadratic;
			uniform float fadeThreshold;
			out vec4 FRAGMENT_COLOR;
			in vec3 fragPos;
			float AlphaFromAttenuation(){
				vec3 fakeViewPos = viewPos;
				fakeViewPos.y = 0;
			    const float distanceToLight = max(max(length(viewPos - fragPos) - fadeThreshold, 0) - viewPos.y, 0);
			    const float attenuation = (linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
			    return 1.0 / attenuation;
			}
			void main(){
				FRAGMENT_COLOR = vec4(color, AlphaFromAttenuation());
			})";

		};
	}
}
