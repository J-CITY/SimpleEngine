#pragma once
#include "editorWindow.h"

#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class ComponentManagerWindow : public EditorWindow {
	public:
		virtual ~ComponentManagerWindow() override = default;
		void draw() override;
	};
}
#endif
