#pragma once
#include "editorWindow.h"
#include <memory>
#include "coreModule/ecs/components/materialRenderer.h"

#ifdef USE_EDITOR
namespace IKIGAI {
	namespace ECS {
		class Object;
		class ComponentBase;
	}
}

namespace IKIGAI::EDITOR {

	void DrawMaterialWidget(ECS::MaterialRenderer::MaterialList& materials);

	class ComponentManagerWindow : public EditorWindow {
	public:
		virtual ~ComponentManagerWindow() override = default;
		void draw() override;
	};

	class ComponentInspectorWindow : public EditorWindow {
	public:
		virtual ~ComponentInspectorWindow() override = default;
		void draw() override;
	private:
		void drawObjectInfo(std::shared_ptr<ECS::Object> selectObject);
		void drawAddComponent(std::shared_ptr<ECS::Object> selectObject);
		void drawComponent(std::shared_ptr<ECS::Object> selectObject, UTILS::WeakPtr<ECS::ComponentBase> component, const std::string& compName, const std::string& typeIdName, const std::string& iconStr);
	};
}
#endif
