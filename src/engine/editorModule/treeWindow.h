#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
#include <memory>

namespace IKIGAI::EDITOR {
	class TreeWindow: public EditorWindow {
		void drawNodeTree();
	public:
		virtual ~TreeWindow() override = default;
		void draw() override;
	};
}
#endif
