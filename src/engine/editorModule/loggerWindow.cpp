#include "loggerWindow.h"
#ifdef USE_EDITOR
#include "sceneModule/sceneManager.h"
#include "misc/cpp/imgui_stdlib.h"

IKIGAI::EDITOR::ImGuiLogger::ImGuiLogger()
{
	editor.SetReadOnly(true); \
	auto lang = TextEditor::LanguageDefinition::Logger();
	editor.SetLanguageDefinition(lang);
	editor.SetPalette(TextEditor::GetLoggerPalette());
}

void IKIGAI::EDITOR::ImGuiLogger::addLog(const TextEditor::Record& record)
{
	mBuffer.push_back(record);
	while (mBuffer.size() > mBufferSize) {
		mBuffer.pop_front();
	}
	editor.SetTextLogLines(mBuffer, mLogLevels, mSearch);
}

void IKIGAI::EDITOR::ImGuiLogger::addLog(UTILS::LOGG::Log::Level level, const std::string& word)
{
	//TODO: replace with fmt
	addLog({
		std::format("{:<8} {:<7.3f} - {}", mLogLevelsToString.at(level), ImGui::GetTime(), word),
		level
	});
}

void IKIGAI::EDITOR::ImGuiLogger::draw()
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	static bool isOpen = true;
	ImGui::Begin("Logger", &isOpen);

	if (ImGui::Button("Clear")) {
		clear();
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("ERROR", &mLogLevels[UTILS::LOGG::Log::Level::Error])) { editor.SetTextLogLines(mBuffer, mLogLevels, mSearch); }
	ImGui::SameLine();
	if (ImGui::Checkbox("DEBUG", &mLogLevels[UTILS::LOGG::Log::Level::Debug])) { editor.SetTextLogLines(mBuffer, mLogLevels, mSearch); }
	ImGui::SameLine();
	if (ImGui::Checkbox("INFO", &mLogLevels[UTILS::LOGG::Log::Level::Info])) { editor.SetTextLogLines(mBuffer, mLogLevels, mSearch); }
	ImGui::SameLine();
	if (ImGui::Checkbox("WARNING", &mLogLevels[UTILS::LOGG::Log::Level::Warning])) { editor.SetTextLogLines(mBuffer, mLogLevels, mSearch); }
	ImGui::SameLine();
	if (ImGui::InputText("Search", &mSearch)) { editor.SetTextLogLines(mBuffer, mLogLevels, mSearch); }

	ImGui::Separator();

	if (ImGui::InputText("Run", &mSearch, ImGuiInputTextFlags_EnterReturnsTrue)) {
		//TODO: run script
	}
	ImGui::Separator();

	ImGui::BeginChild("##logscrolling");
	editor.Render("TextEditor");
	ImGui::EndChild();

	ImGui::End();
}

void IKIGAI::EDITOR::LoggerWindow::draw()
{
	static ImGuiLogger log;

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Example: Log");
	if (ImGui::SmallButton("[Debug] Add 5 entries")) {
		static int counter = 0;
		const UTILS::LOGG::Log::Level categories[4] = {UTILS::LOGG::Log::Level::Debug, UTILS::LOGG::Log::Level::Info, UTILS::LOGG::Log::Level::Error, UTILS::LOGG::Log::Level::Warning};
		const std::string words[] = {"Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Paucaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaailoquent"};
		for (int n = 0; n < 5; n++) {
			auto category = categories[counter % IM_ARRAYSIZE(categories)];
			const auto& word = words[counter % IM_ARRAYSIZE(words)];
			log.addLog(category, word);
			counter++;
		}
	}
	ImGui::End();
	log.draw();
}
#endif
