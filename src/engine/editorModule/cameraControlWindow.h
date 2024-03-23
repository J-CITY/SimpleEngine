#pragma once
#include "editorWindow.h"

#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class CameraControlWindow : public EditorWindow {
	public:
		virtual ~CameraControlWindow() override = default;
		void draw() override;
	};
}
#endif
