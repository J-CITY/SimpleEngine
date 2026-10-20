#include "coreModule/core/application.h"
#ifndef OCULUS
#ifdef USE_SDL
#include <SDL3/SDL_main.h>
#endif

int main(int argc, char* argv[]) {
	return IKIGAI::CORE::RunApplication(argc, argv);
}
#endif
