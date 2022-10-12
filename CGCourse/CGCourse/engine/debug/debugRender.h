#pragma once

namespace KUMA
{
	namespace CORE_SYSTEM
	{
		class Core;
	}
}

namespace KUMA::DEBUG {
	class DebugRender {

	public:
		DebugRender();
		~DebugRender();

		void draw(CORE_SYSTEM::Core& core);
	private:
		void drawWindowWidget(CORE_SYSTEM::Core& core);
	};
}
