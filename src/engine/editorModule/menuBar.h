#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class MenuBar: public EditorWindow{
	public:
		virtual ~MenuBar() override = default;
		void draw() override;
	};
}

#endif
