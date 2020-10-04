#pragma once
#include <memory>
#include <vector>
#include <GL/glew.h>



#include "render.h"
#include "../math.h"
#include "components/perspectiveCamera.h"

namespace SE {

	class GlManager {
	public:
		GlManager(const Mathgl::Vec2& windowSize);
		~GlManager();

		void setDrawSize(const Mathgl::Vec2& size);

		void bindRenderTarget() const;
		void renderScene(Object* obj);
		void setActiveCamera(std::shared_ptr<Camera> camera);

		//void addDirectionalLight(std::shared_ptr<DirectionalLight> light);
		//void addPointLight(std::shared_ptr<PointLight> light);
		//void addSpotLight(std::shared_ptr<SpotLight> light);

		//void removeDirectionalLight(std::shared_ptr<DirectionalLight> light);
		//void removePointLight(std::shared_ptr<PointLight> light);
		//void removeSpotLight(std::shared_ptr<SpotLight> light);

		Mathgl::Mat4 getViewMatrix();
		Mathgl::Mat4 getProjectionMatrix();

		void draw(Object* entity);
		//void drawLine(Line line);

		int width, height;

		GLuint lineBuffer;
		GLuint VertexArrayID;

	private:
		std::unique_ptr<Render> render;
		//std::unique_ptr<SimpleRender> simpleRenderer;

		std::shared_ptr<Camera> activeCamera;

		//std::vector<std::shared_ptr<DirectionalLight>> m_directionalLights;
		//std::vector<std::shared_ptr<PointLight>> m_pointLights;
		//std::vector<std::shared_ptr<SpotLight>> m_spotLights;
	};
}
