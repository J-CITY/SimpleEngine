#pragma once
#ifdef USE_EDITOR
#include "editorWindow.h"

namespace IKIGAI::EDITOR {
	class MaterialWindow : public EditorWindow {
	public:
		virtual ~MaterialWindow() override = default;
		void draw() override;
	};
}
#endif
