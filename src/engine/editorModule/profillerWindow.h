#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class ProfillerWindow : public EditorWindow {
	public:
		virtual ~ProfillerWindow() override = default;
		void draw() override;
	};
}
#endif