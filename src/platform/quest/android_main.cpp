#include "coreModule/core/application.h"
#include <android_native_app_glue.h>

void android_main(android_app* app) {
	IKIGAI::CORE::RunApplication(0, nullptr, { app });
}
