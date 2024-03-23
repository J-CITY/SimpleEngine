#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class StatWindow : public EditorWindow {
	public:
		virtual ~StatWindow() override = default;
		void draw() override;
	};
}
#endif
