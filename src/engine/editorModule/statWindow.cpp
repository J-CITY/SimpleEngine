#include "statWindow.h"
#ifdef USE_EDITOR
#include "editorRender.h"
#include "IconsFontAwesome5.h"
#include "imgui.h"
#include "renderModule/backends/gl/materialGl.h"
#include "resourceModule/materialManager.h"

#include "sceneModule/sceneManager.h"
#include "misc/cpp/imgui_stdlib.h"
#include "resourceModule/textureManager.h"
#include "utilsModule/imguiHelper/imguiWidgets.h"
#include "utilsModule/time/time.h"


void IKIGAI::EDITOR::StatWindow::draw() {
	static std::vector<float> values(100, 0.0f);
	static int valuesOffset = 0;
	static float updateTime = 0;
	static float timeToUpdate = 0;
	static double fps = 0.0;
	static double dt;

	ImGui::Begin("Stats");
	auto& timer = TIME::Timer::GetInstance();
	if (timeToUpdate <= 0.0) {
		fps = timer.getFPS();
		dt = timer.getDeltaTimeUnscaled().count();
		values[valuesOffset] = fps;
		valuesOffset = (valuesOffset + 1) % values.size();
		timeToUpdate = updateTime;
	} else {
		timeToUpdate -= timer.getDeltaTimeUnscaled().count();
	}
	if (IMGUI::SliderFloatWithSteps("Update time", &updateTime, 0.0f, 1.0f, 0.10f)) {
		timeToUpdate = updateTime;
	}
	ImGui::Text("FPS: %f", fps);
	ImGui::Text("Delta: %f", dt);
	ImGui::PlotLines("##FPSGraph", values.data(), values.size(), valuesOffset, nullptr, -10.0f, 500.0f, ImVec2(0, 80.0f));
	ImGui::End();
}
#endif
