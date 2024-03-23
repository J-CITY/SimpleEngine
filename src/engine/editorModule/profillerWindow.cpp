#include "profillerWindow.h"
#ifdef USE_EDITOR
#ifndef OCULUS
#include "imgui.h"
#endif

#include "sceneModule/sceneManager.h"
#include "utilsModule/imguiHelper/imgui_widget_flamegraph.h"
#include "utilsModule/profiler/profiler.h"

int frameId = 0;
std::thread::id frameThreadId;


static void ProfilerValueGetter(float* startTimestamp, float* endTimestamp, ImU8* level, const char** caption, const void* _data, int idx) {
	auto& reports = IKIGAI::PROFILER::Profiler::getReportHistory();

	auto& frame = reports[frameId];
	auto& data = frame.mCallsTree.at(frameThreadId);

	if (caption) *caption = data[idx].mName.c_str();
	if (level) *level = data[idx].level;
	if (startTimestamp) *startTimestamp = data[idx].mStart;
	if (endTimestamp) *endTimestamp = data[idx].mStart + data[idx].mDuration;
}


void IKIGAI::EDITOR::ProfillerWindow::draw() {

	static bool isOpen = true;
	auto& reports = PROFILER::Profiler::getReportHistory();

	ImGui::Begin("Profiler Window", &isOpen);
	static bool isProfileOn = false;
	if (ImGui::Checkbox("Is Enabled", &isProfileOn)) {
		PROFILER::Profiler::ToggleEnable();
		//PROFILER::Profiler::ClearHistory();
	}
	if (/*!isProfileOn ||*/ reports.size() < frameId + 1) {
		ImGui::Text("Profiler is off");
		ImGui::End();
		return;
	}

	auto fid = ImGuiWidgetFlameGraph::PlotHistogramPressed("Histogram", PROFILER::Profiler::getReportHistoryDurations().data(),
		PROFILER::Profiler::getReportHistoryDurations().size(), 0, NULL, 0.001f, 1.0f, ImVec2(400, 80.0f));
	if (fid >= 0) {
		frameId = fid;
	}

	auto& frame = reports[frameId];
	for (auto& e : frame.mCallsTree) {
		frameThreadId = e.first;
		ImGuiWidgetFlameGraph::PlotFlame("CPU", &ProfilerValueGetter, nullptr, e.second.size(), 0, "Main Thread", FLT_MAX, FLT_MAX, ImVec2(400, 0));
		break;
	}
	ImGui::End();
}
#endif
