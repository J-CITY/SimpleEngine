#pragma once
#ifdef USE_EDITOR
namespace IKIGAI::EDITOR {
	class EditorWindow {
	protected:
		bool mIsOpen = true;
	public:
		virtual ~EditorWindow() = default;
		virtual void draw() = 0;

		[[nodiscard]] bool isOpen() const { return mIsOpen; }
		void setOpen(bool b) {
			mIsOpen = b;
		}
	};
}
#endif
