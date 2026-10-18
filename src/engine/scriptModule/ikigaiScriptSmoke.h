#pragma once
#if USE_IKIGAI_SCRIPT
namespace IKIGAI::CORE {
class App;
}
namespace IKIGAI::SCRIPTING {
void RunIkigaiScriptSmoke(CORE::App& app);
}
#endif
