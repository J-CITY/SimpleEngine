#pragma once

#include <memory>
#include <string>
#include "render.h"

namespace IKIGAI {
	namespace RESOURCES {
		class Shader;
	}
	namespace MATHGL {
		struct Vector3;
		struct Matrix4;
	}
}

namespace IKIGAI {
	namespace RENDER {
		//class PrimitiveRender {
		//public:
		//	PrimitiveRender(BaseRender& render);
		//	~PrimitiveRender();
		//	void setViewProjection(const MATHGL::Matrix4& p_viewProjection);
		//	void drawLine(const MATHGL::Vector3& start, const MATHGL::Vector3& end, const MATHGL::Vector3& color, float lineWidth = 1.0f);
		//	void drawGrid(const MATHGL::Vector3& viewPos, const MATHGL::Vector3& color, int32_t gridSize = 50, float linear = 0.0f, float quadratic = 0.0f, float fadeThreshold = 0.0f, float p_lineWidth = 1.0f);
		//private:
		//	std::shared_ptr<RESOURCES::Shader> lineShader = nullptr;
		//	std::shared_ptr<RESOURCES::Shader> gridShader = nullptr;
		//	std::shared_ptr<RESOURCES::Mesh> lineMesh = nullptr;
		//	BaseRender& render;
		//
		//	inline const static std::string vertexLineShader = R"(
		//	#version 460 core
		//	uniform vec3 start;
		//	uniform vec3 end;
		//	uniform mat4 viewProjection;
		//	void main(){
		//		vec3 position = gl_VertexID == 0 ? start : end;
		//	    gl_Position = viewProjection * vec4(position, 1.0);
		//	})";
		//
		//	inline const static std::string fragmentLineShader = R"(
		//	#version 460 core
		//	uniform vec3 color;
		//	out vec4 FRAGMENT_COLOR;
		//	void main() {
		//		FRAGMENT_COLOR = vec4(color, 1.0);
		//	})";
		//
		//
		//	inline const static std::string vertexGridShader = R"(
		//	#version 460 core
		//	uniform vec3 start;
		//	uniform vec3 end;
		//	uniform mat4 viewProjection;
		//	out vec3 fragPos;
		//	void main(){
		//		vec3 position = gl_VertexID == 0 ? start : end;
		//		fragPos = position;
		//	    gl_Position = viewProjection * vec4(position, 1.0);
		//	})";
		//
		//	inline const static std::string fragmentGridShader = R"(
		//	#version 460 core
		//	uniform vec3 color;
		//	uniform vec3 viewPos;
		//	uniform float linear;
		//	uniform float quadratic;
		//	uniform float fadeThreshold;
		//	out vec4 FRAGMENT_COLOR;
		//	in vec3 fragPos;
		//	float AlphaFromAttenuation(){
		//		vec3 fakeViewPos = viewPos;
		//		fakeViewPos.y = 0;
		//	    const float distanceToLight = max(max(length(viewPos - fragPos) - fadeThreshold, 0) - viewPos.y, 0);
		//	    const float attenuation = (linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));
		//	    return 1.0 / attenuation;
		//	}
		//	void main(){
		//		FRAGMENT_COLOR = vec4(color, AlphaFromAttenuation());
		//	})";
		//
		//};
	}
}
