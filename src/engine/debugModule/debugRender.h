#pragma once
#include <memory>

#ifdef USE_EDITOR_
namespace IKIGAI
{
	namespace ECS
	{
		class Object;
	}
}

namespace IKIGAI
{
	namespace RENDER
	{
		class MaterialGl;
	}
}

namespace IKIGAI
{
	namespace CORE_SYSTEM
	{
		class Core;
	}
}

namespace IKIGAI::DEBUG {
	class DebugRender {

	public:
		inline static std::unique_ptr<ECS::Object> debugCamera;

		DebugRender();
		~DebugRender();
#ifdef VULKAN_BACKEND
		void initForVk();
#endif
		void draw(CORE_SYSTEM::Core& core);
		void postDraw();
	private:
		void drawWindowWidget(CORE_SYSTEM::Core& core);
		void drawMaterialWidget(RENDER::MaterialGl* material);
		//void widgetMaterial(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop);
		void drawComponentInspector();
		void drawTextureWatcher();
		void drawScene();
		void drawEditorScene();
		void drawStats();
		void drawPopup();
		void drawMainWindow();
		void drawConsole();
		void drawProfiler();
	};
}
#endif
