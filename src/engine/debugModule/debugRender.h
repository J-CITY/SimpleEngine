#pragma once


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
		DebugRender();
		~DebugRender();
#ifdef VULKAN_BACKEND
		void initForVk();
#endif
		void draw(CORE_SYSTEM::Core& core);
	private:
		void drawWindowWidget(CORE_SYSTEM::Core& core);
		void drawMaterialWidget(RENDER::MaterialGl* material);
		//void widgetMaterial(UTILS::WeakPtr<ECS::Component> component, const rttr::property& prop);
		void drawComponentInspector();
		void drawTextureWatcher();
		void drawScene();
		void drawStats();
		void drawPopup();
		void drawMainWindow();
	};
}
