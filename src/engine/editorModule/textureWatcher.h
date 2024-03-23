#pragma once
#include "editorWindow.h"
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class TextureWatcherWindow : public EditorWindow {
	public:
		virtual ~TextureWatcherWindow() override = default;
		void draw() override;
	};
}
#endif
