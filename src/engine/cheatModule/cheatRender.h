#pragma once

#ifdef USE_CHEATS
namespace IKIGAI::CHEATS {
	class CheatRender {
	public:
		CheatRender();

		virtual ~CheatRender() = default;
		void draw();
	};
}
#endif
