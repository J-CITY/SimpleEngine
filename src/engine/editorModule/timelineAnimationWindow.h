#pragma once
#include <string>

#include "editorWindow.h"
#include "utilsModule/animation.h"

#ifdef USE_EDITOR

#include <memory>


namespace IKIGAI
{
	namespace ANIMATION
	{
		class Animation;
	}
}

namespace IKIGAI::EDITOR {
	class TimelineAnimationWindow : public EditorWindow {
		std::unique_ptr<IKIGAI::ANIMATION::Animation> mAnimation;

		IKIGAI::ANIMATION::TimelineAnimationDescriptor toDescriptor();
		std::unique_ptr<IKIGAI::ANIMATION::Animation> fromDescriptor(IKIGAI::ANIMATION::TimelineAnimationDescriptor& desc);

	public:
		TimelineAnimationWindow();
		void openResource(const std::string& path);
		virtual ~TimelineAnimationWindow() override = default;
		void draw() override;
	};
}
#endif
