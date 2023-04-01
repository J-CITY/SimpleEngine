#include "debugRender.h"

#include <memory>
#include <serdepp/include/serdepp/adaptor/reflection.hpp>


#include "../render/backends/interface/driverInterface.h"

#ifdef VULKAN_BACKEND
#include <vulkan/vulkan_core.h>
#include "imgui/imgui_impl_vulkan.h"
#include "../render/backends/vk/driverVk.h"
#include "../render/backends/vk/frameBufferVk.h"
#endif

#include "../render/gameRendererGl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"

#ifdef OPENGL_BACKEND
#include "imgui/imgui_impl_opengl3.h"
#include "../render/backends/gl/materialGl.h"
#endif
#include <unordered_set>

#include "../window/window.h"
#include "../inputManager/inputManager.h"

using namespace KUMA;
using namespace KUMA::DEBUG;

#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "../ecs/object.h"
#include "../gui/guiObject.h"
#include "../scene/sceneManager.h"
#include "../core/core.h"
#include "../utils/time/time.h"


int uniqueNodeId = 0;
std::shared_ptr<KUMA::ECS::Object> selectObj;
std::shared_ptr<KUMA::GUI::GuiObject> selectObjGui;


struct AnimationLineInfo {
	ObjectId<ECS::Object> id;
	std::string name;
	std::string componentName;
};
std::vector<AnimationLineInfo> SequencerItemTypeNames{};

std::shared_ptr<KUMA::ECS::Object> recursiveDraw(KUMA::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<KUMA::ECS::Object> parentEntity) {
	std::shared_ptr<KUMA::ECS::Object> selectedNode;

	std::span<std::shared_ptr<KUMA::ECS::Object>> nodeList;

	if (parentEntity) {
		nodeList = parentEntity->getChildren();
	}
	else {
		nodeList = activeScene.getObjects();
	}
	auto i = 0u;
	for (auto node : nodeList) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;

		bool isParent = node->getChildren().size();

		if (!isParent) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		const auto name = node->getName();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->getName().c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {

			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(KUMA::ECS::Object*));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);

				//change node parent
				auto parent = other->getParent();
				if (parent) {
					//parent->removeChild(other);
					//other->setParent(nullptr);
				}
				//node.second->addChild(other);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedNode = node;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::InvisibleButton("__NODE_ORDER_SET__", { -1, 5 });
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);
				if (node.get() != other && node->getParent()) {
					auto parent = other->getParent();
					if (parent) {
						//parent->removeChild(other);
						//other->setParent(nullptr);
					}
					//node.second->getParent()->addChild(other->getID(), other, i + 1);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isParent && nodeIsOpen) {
			auto childClickedEntity = recursiveDraw(activeScene, node);
			if (!selectedNode) {
				selectedNode = childClickedEntity;
			}
			ImGui::TreePop();
		}
		i++;
		ImGui::PopID();
	}
	return selectedNode;
}
void drawNodeTree(KUMA::CORE_SYSTEM::Core& core) {
	static bool isobjTreeOpen = true;
	if (core.sceneManager->hasCurrentScene()) {
		auto& scene = core.sceneManager->getCurrentScene();
		if (ImGui::Begin("Scene Hierarchy", &isobjTreeOpen)) {
			uniqueNodeId = 0;
			auto _selectNode = recursiveDraw(scene, nullptr);
			if (_selectNode) {
				selectObj = _selectNode;
				if (selectObj) {
					auto components = ECS::ComponentManager::getInstance()->getComponents(selectObj->getID());
					for (auto& e : components) {
						SequencerItemTypeNames.push_back(AnimationLineInfo{ selectObj->getID(), selectObj->getName(), e->getName() });
					}
				}
			}
		}
		ImGui::End();
	}
}

std::optional<unsigned> ImGuiLoadTextureFromFile(const std::string& filename) {
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = RESOURCES::stbiLoad(filename.c_str(), &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return std::nullopt;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	RESOURCES::stbiImageFree(image_data);

	return image_texture;
}

struct FileBrowser {
	std::string mPath;
	std::string mSelectedFolderPath;
	int elementSize = 100;
	FileBrowser(const std::string& path): mPath(path), mSelectedFolderPath(path) {
		textureCache["__image__"] = ImGuiLoadTextureFromFile(UTILS::getRealPath("Textures/Debug/Editor/image-white.png")).value();
		textureCache["__dir__"] = ImGuiLoadTextureFromFile(UTILS::getRealPath("Textures/Debug/Editor/folder-white.png")).value();
		textureCache["__file__"] = ImGuiLoadTextureFromFile(UTILS::getRealPath("Textures/Debug/Editor/file-white.png")).value();
		textureCache["__font__"] = ImGuiLoadTextureFromFile(UTILS::getRealPath("Textures/Debug/Editor/font-white.png")).value();
		textureCache["__object__"] = ImGuiLoadTextureFromFile(UTILS::getRealPath("Textures/Debug/Editor/cube-white.png")).value();
	}
	int globalNodeId = 0;
	void draw() {
		//ImGui::Columns(2);

		ImGui::Begin("File Browser");
		globalNodeId = 0;
		//for (const auto& entry : std::filesystem::directory_iterator(mPath)) {
		//	draw(entry.path().string());
		//}
		drawItem(globalNodeId, mPath);
		ImGui::End();
		//ImGui::NextColumn();
		ImGui::Begin("Folder");
		drawFolder(mSelectedFolderPath);
		ImGui::End();

		//ImGui::NextColumn();
		//ImGui::Columns(1);
	}
private:
	enum class FileType {
		DIR,
		IMAGE,
		MODEL,
		TEXT,
		FONT
	};

	std::unordered_map<std::string, unsigned> textureCache;

	std::unordered_set<std::string> imageExt = { ".png", ".jpg", ".jpeg" };
	std::unordered_set<std::string> objExt = { ".obj", ".fbx", ".dae"};
	std::unordered_set<std::string> fontExt = { ".ttf" };
	FileType getFileType(const std::string& ext) {
		if (ext == "dir") {
			return FileType::DIR;
		}
		if (imageExt.contains(ext)) {
			return FileType::IMAGE;
		}
		if (fontExt.contains(ext)) {
			return FileType::TEXT;
		}
		return FileType::TEXT;
	}

	void drawFolder(std::string_view path) {
		ImGui::BeginChild("##FolderBar", ImVec2(0, 30));
		ImGui::Text(std::string(path).c_str());
		int sliderSize = 200;
		ImGui::SameLine(ImGui::GetWindowWidth() - sliderSize);
		ImGui::PushItemWidth(sliderSize);
		ImGui::SliderInt("##FSize", &elementSize, 20, 150);
		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::BeginChild("##FolderFiles", ImVec2(0, 0));
		const auto winSize = ImGui::GetWindowSize();
		//const float col0Size = ImGui::GetColumnWidth(0);
		//const float col1Size = winSize.x - col0Size;
		//ImGui::SetColumnWidth(1, col1Size);
		auto cols = static_cast<int>(winSize.x / (elementSize + 15));
		if (cols <= 0) {
			cols = 1;
		}
		//ImGui::Columns(static_cast<int>(col1Size / elementSize));
		int i = 0;
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			bool isDirectory = std::filesystem::is_directory(entry);
			auto ext = isDirectory ? "dir" : entry.path().extension().string();
			auto extType = getFileType(ext);
			ImGui::BeginGroup();
			std::string imPath;
			switch (extType) {
			case FileType::DIR: imPath = "__dir__";  break;
			case FileType::IMAGE: {
				imPath = entry.path().string();
				if (!textureCache.contains(imPath)) {
					textureCache[imPath] = ImGuiLoadTextureFromFile(imPath).value();
				}
				//imPath = "__image__";
				break;
			};
			case FileType::MODEL: imPath = "__object__"; break;
			case FileType::TEXT: imPath = "__file__"; break;
			case FileType::FONT: imPath = "__font__"; break;
			default: ;
			}
			if (ImGui::ImageButton((void*)textureCache.at(imPath), { static_cast<float>(elementSize), static_cast<float>(elementSize) }, ImVec2(0, 1), ImVec2(1, 0)) && isDirectory) {
				mSelectedFolderPath = entry.path().string();
			}
			
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
			ImGui::Text(entry.path().filename().string().c_str());
			ImGui::PopTextWrapPos();

			ImGui::EndGroup();
			i = (i + 1) % cols;
			if (i) {
				ImGui::SameLine();
			}
			//ImGui::NextColumn();
		}
		ImGui::EndChild();
		//ImGui::Columns(2);
	}

	void draw(std::string_view path) {
		bool isDirectory = std::filesystem::is_directory(path);
		if (isDirectory) {
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				drawItem(globalNodeId, entry.path().string());
				globalNodeId++;
			}
		}
	}

	void drawItem(int i, std::string_view path) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;

		bool isDirectory = std::filesystem::is_directory(path);
		if (!isDirectory) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_None;
		}
		std::string _path = std::string(path.begin(), path.end());
		if (path.back() == '\\' || path.back() == '/') {
			_path = std::filesystem::path(path).parent_path().string();
		}
		const auto name = std::filesystem::path(_path).filename().string();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(name.c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
		//if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
		//	if (ImGui::MenuItem("Create new")) {
		//
		//	}
		//	if (ImGui::MenuItem("Delete")) {
		//
		//	}
		//
		//	ImGui::EndPopup();
		//}
		//ImGui::PopStyleVar();

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
		//if (ImGui::BeginDragDropSource()) {
		//	ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(KUMA::ECS::Object*));
		//	ImGui::TextUnformatted(name.c_str());
		//	ImGui::EndDragDropSource();
		//}
		//ImGui::PopStyleVar();

		//if (ImGui::BeginDragDropTarget()) {
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
		//		auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);
		//
		//		//change node parent
		//		auto parent = other->getParent();
		//		if (parent) {
		//			//parent->removeChild(other);
		//			//other->setParent(nullptr);
		//		}
		//		//node.second->addChild(other);
		//	}
		//	ImGui::EndDragDropTarget();
		//}

		if (isDirectory && ImGui::IsItemClicked()) {
			mSelectedFolderPath = _path;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::InvisibleButton("__NODE_ORDER_SET__", { -1, 5 });
		ImGui::PopStyleVar();

		//if (ImGui::BeginDragDropTarget()) {
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
		//		auto other = *static_cast<KUMA::ECS::Object**>(payload->Data);
		//		if (node.get() != other && node->getParent()) {
		//			auto parent = other->getParent();
		//			if (parent) {
		//				//parent->removeChild(other);
		//				//other->setParent(nullptr);
		//			}
		//			//node.second->getParent()->addChild(other->getID(), other, i + 1);
		//		}
		//	}
		//	ImGui::EndDragDropTarget();
		//}

		if (isDirectory && nodeIsOpen) {
			draw(path);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
};

std::unique_ptr<FileBrowser> fileBrowser;

/*
std::shared_ptr<KUMA::GUI::GuiObject> recursiveDrawGui(KUMA::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<KUMA::GUI::GuiObject> parentEntity) {
	std::shared_ptr<KUMA::GUI::GuiObject> selectedNode;

	std::vector<std::shared_ptr<KUMA::GUI::GuiObject>> nodeList;

	if (parentEntity) {
		nodeList = parentEntity->childs;
	}
	else {
		nodeList = activeScene.guiObjs;
	}
	auto i = 0u;
	for (auto node : nodeList) {
		ImGui::PushID(("node_" + std::to_string(i)).c_str());
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Selected;

		bool isParent = node->childs.size();

		if (!isParent) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		const auto name = node->name;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->name.c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 5.0f, 5.0f });
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {

			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("__SCENE_NODE_DRAG__", &node, sizeof(KUMA::ECS::Object*));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::GUI::GuiObject**>(payload->Data);

				//change node parent
				auto parent = other->parent;
				if (parent) {
					//parent->removeChild(other);
					//other->setParent(nullptr);
				}
				//node.second->addChild(other);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked()) {
			selectedNode = node;
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
		ImGui::InvisibleButton("__NODE_ORDER_SET__", { -1, 5 });
		ImGui::PopStyleVar();

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("__SCENE_NODE_DRAG__")) {
				auto other = *static_cast<KUMA::GUI::GuiObject**>(payload->Data);
				if (node.get() != other && node->parent) {
					auto parent = other->parent;
					if (parent) {
						//parent->removeChild(other);
						//other->setParent(nullptr);
					}
					//node.second->getParent()->addChild(other->getID(), other, i + 1);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (isParent && nodeIsOpen) {
			auto childClickedEntity = recursiveDrawGui(activeScene, node);
			if (!selectedNode) {
				selectedNode = childClickedEntity;
			}
			ImGui::TreePop();
		}
		i++;
		ImGui::PopID();
	}
	return selectedNode;
}
*/
//void drawNodeTreeGui(KUMA::CORE_SYSTEM::Core& core) {
//	static bool isobjTreeOpen = true;
//	if (core.sceneManager->hasCurrentScene()) {
//		auto& scene = core.sceneManager->getCurrentScene();
//		if (ImGui::Begin("Scene Hierarchy", &isobjTreeOpen)) {
//			uniqueNodeId = 0;
//			auto _selectNode = recursiveDrawGui(scene, nullptr);
//			if (_selectNode) {
//				selectObjGui = _selectNode;
//			}
//		}
//		ImGui::End();
//	}
//}


template<class T>
bool drawWidget(std::string name, T& elem) {
	using type = std::remove_reference_t<decltype(elem)>;
	int a=1;
	return false;
}

template<>
bool drawWidget(std::string name, float& elem) {
	return ImGui::DragFloat(name.c_str(), &elem, 0.1f, 0.1f, 500.0f);
}
template<>
bool drawWidget(std::string name, int& elem) {
	return ImGui::DragInt(name.c_str(), &elem, 1.0f, 0, 100);
}
template<>
bool drawWidget(std::string name, bool& elem) {
	return ImGui::Checkbox(name.c_str(), &elem);
}
template<>
bool drawWidget(std::string name, std::string& elem) {
	return false;
}
template<>
bool drawWidget(std::string name, MATHGL::Vector2u& elem) {
	int arr[2];
	arr[0] = elem.x;
	arr[1] = elem.y;
	auto b = ImGui::DragInt2(name.c_str(), arr, 1, 0, 4000);
	elem.x = arr[0];
	elem.y = arr[1];
	return b;
}

template<std::size_t I = 0, typename T>
inline std::enable_if < I == serde::tuple_size_v<T>, bool>::type go(const serde::serde_struct_info<T>& t, T& val) {
	return true;
}

template<std::size_t I = 0, typename T>
inline std::enable_if < I < serde::tuple_size_v<T>, bool>::type go(const serde::serde_struct_info<T>& t, T& val) {
	auto b = drawWidget(std::string(t.member_info<I>(val).name()), t.member_info<I>(val).value());
	return b || go<I + 1, T>(t, val);
}

template<typename T>
bool buildWidget(T& data) {
	constexpr auto info = serde::type_info<T>;
	return go(info, data);
}

struct DebugConfig {
	enum class WidgetType {
		WINDOW = 0
	};

	static bool check(WidgetType t) { return conf[static_cast<int>(t)]; }
	inline static std::bitset<10> conf;
};

void ThemeNord(ImGuiStyle* dst = nullptr) {
	// Nord/Nordic GTK [Improvised]
	// https://github.com/EliverLara/Nordic
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	ImGui::StyleColorsDark(style);//Reset to base/dark theme
	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.87f, 0.91f, 0.88f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.49f, 0.50f, 0.53f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.23f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.09f, 0.09f, 0.09f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.56f, 0.74f, 0.73f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.53f, 0.75f, 0.82f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.23f, 0.26f, 0.32f, 0.60f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.23f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.63f, 0.76f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.51f, 0.63f, 0.76f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.51f, 0.63f, 0.76f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.53f, 0.75f, 0.82f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56f, 0.74f, 0.73f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.53f, 0.75f, 0.82f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.53f, 0.75f, 0.82f, 0.86f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.61f, 0.74f, 0.87f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.24f, 0.31f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.26f, 0.32f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.19f, 0.23f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.56f, 0.74f, 0.73f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.53f, 0.75f, 0.82f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.37f, 0.51f, 0.67f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.53f, 0.75f, 0.82f, 0.86f);
	style->WindowBorderSize = 1.00f;
	style->ChildBorderSize = 1.00f;
	style->PopupBorderSize = 1.00f;
	style->FrameBorderSize = 1.00f;
}

void DarkTheme(ImGuiStyle* dst = nullptr) {
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	style->WindowMinSize = ImVec2(160, 20);
	style->FramePadding = ImVec2(4, 2);
	style->ItemSpacing = ImVec2(6, 2);
	style->ItemInnerSpacing = ImVec2(6, 4);
	style->Alpha = 0.95f;
	style->WindowRounding = 4.0f;
	style->FrameRounding = 2.0f;
	style->IndentSpacing = 6.0f;
	style->ItemInnerSpacing = ImVec2(2, 4);
	style->ColumnsMinSpacing = 50.0f;
	style->GrabMinSize = 14.0f;
	style->GrabRounding = 16.0f;
	style->ScrollbarSize = 12.0f;
	style->ScrollbarRounding = 16.0f;
	
	style->Colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);
}

void LightTheme(ImGuiStyle* dst = nullptr) {
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->Clear();
	//io.Fonts->AddFontFromFileTTF(ofToDataPath("fonts/OpenSans-Light.ttf", true).c_str(), 16);
	//io.Fonts->AddFontFromFileTTF(ofToDataPath("fonts/OpenSans-Regular.ttf", true).c_str(), 16);
	//io.Fonts->AddFontFromFileTTF(ofToDataPath("fonts/OpenSans-Light.ttf", true).c_str(), 32);
	//io.Fonts->AddFontFromFileTTF(ofToDataPath("fonts/OpenSans-Regular.ttf", true).c_str(), 11);
	//io.Fonts->AddFontFromFileTTF(ofToDataPath("fonts/OpenSans-Bold.ttf", true).c_str(), 11);
	//io.Fonts->Build();
	

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.39f, 0.38f, 0.77f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.92f, 0.91f, 0.88f, 0.70f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.58f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.92f, 0.91f, 0.88f, 0.92f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.84f, 0.83f, 0.80f, 0.65f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.99f, 1.00f, 0.40f, 0.78f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.47f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.21f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.90f, 0.91f, 0.00f, 0.78f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 1.00f, 0.00f, 0.80f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 0.14f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.99f, 1.00f, 0.22f, 0.86f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.25f, 1.00f, 0.00f, 0.76f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 1.00f, 0.00f, 0.86f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 0.32f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.04f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 1.00f, 0.00f, 0.78f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	//style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	//style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	//style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

DebugRender::DebugRender() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		initForVk();
		auto win = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getGLFWWin();
		ImGui_ImplGlfw_InitForVulkan(win, true);
	}
#endif
	DarkTheme();

	// Setup Platform/Renderer backends
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		auto& window = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getContext();
		ImGui_ImplGlfw_InitForOpenGL(&window, true);
		const char* glsl_version = "#version 330";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}
#endif

#ifdef OPENGL_BACKEND || VULKAN_BACKEND
	RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyEvent.add([](GLFWwindow* window, int key, int scancode, int action, int mods){
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});
	RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseButtonEvent.add([](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	});
#endif

	fileBrowser = std::make_unique<FileBrowser>(Config::ROOT + Config::ASSETS_PATH);
}
#ifdef VULKAN_BACKEND
#include "../render/gameRendererVk.h"
void DebugRender::initForVk() {
	auto render = reinterpret_cast<RENDER::GameRendererVk&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).getDriver();

	auto rd = render->GetRenderData();

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = render->m_VulkanInstance;
	init_info.PhysicalDevice = render->m_MainDevice.PhysicalDevice;
	init_info.Device = render->m_MainDevice.LogicalDevice;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.Allocator = nullptr;
	init_info.QueueFamily = rd.graphic_queue_index;
	init_info.Queue = rd.graphic_queue;
	init_info.DescriptorPool = rd.imgui_descriptor_pool;
	init_info.MinImageCount = rd.min_image_count;
	init_info.ImageCount = rd.image_count;
	init_info.CheckVkResultFn = nullptr;


	ImGui_ImplVulkan_Init(&init_info, render->defaultFb->m_RenderPass);

	VkCommandPool command_pool = render->m_CommandHandler.GetCommandPool();

	VkCommandBuffer command_buffer = render->m_CommandHandler.GetCommandBuffer(0);

	vkResetCommandPool(rd.device, command_pool, 0);
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &command_buffer;
	vkEndCommandBuffer(command_buffer);
	vkQueueSubmit(rd.graphic_queue, 1, &end_info, VK_NULL_HANDLE);
	vkDeviceWaitIdle(rd.device);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}
#endif

DebugRender::~DebugRender() {
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
#endif
}

void DebugRender::drawWindowWidget(CORE_SYSTEM::Core& core) {
	if (DebugConfig::check(DebugConfig::WidgetType::WINDOW)) {
		ImGui::Begin("Window Config");
		auto b = buildWidget(core.window->getSetting());
		if (b) {
			core.window->updateWindow();
		}
		ImGui::End();
	}
}

bool drawFloatN(std::string& name, int size, float* vec, float step = 0.1f) {
	switch (size) {
	case 2:
		return ImGui::DragFloat2(name.c_str(), vec, step);
	case 3:
		return ImGui::DragFloat3(name.c_str(), vec, step);
	case 4:
		return ImGui::DragFloat4(name.c_str(), vec, step);
	default: assert(false, "Error");
	}
	return false;
}

bool drawColorN(std::string& name, int size, float* vec) {
	switch (size) {
	case 3:
		return ImGui::ColorEdit3(name.c_str(), vec);
	case 4:
		return ImGui::ColorEdit4(name.c_str(), vec);
	default: assert(false, "Error");
	}
	return false;
}

class CombineVecEdit {
	size_t mVecSize = 3;
	std::string mName;
	bool mIsColorMode = false;
	std::string mButtonLblColor;
	std::string mButtonLblVec;
	float mWidth = 200.0f;
public:
	enum class MODE {
		COMBINE, COLOR, POS
	};
	MODE mMode = MODE::COMBINE;
	CombineVecEdit(std::string_view name, size_t vecSize, MODE mode = MODE::COMBINE): mVecSize(vecSize), mName(name), mMode(mode) {
		switch (mVecSize) {
		case 2: mButtonLblVec = "XY"; mButtonLblColor = "RG"; mMode = MODE::POS; break;
		case 3: mButtonLblVec = "XYZ"; mButtonLblColor = "RGB"; mWidth = 150.0f; break;
		case 4: mButtonLblVec = "XYZW"; mButtonLblColor = "RGBA"; mWidth = 150.0f; break;
		default: assert(false, "Support only 2, 3, 4 vec size");
		}
		if (mode == MODE::POS) {
			mIsColorMode = false;
		}
		if (mode == MODE::COLOR) {
			mIsColorMode = true;
		}
	}
	inline static std::unordered_map<std::string, CombineVecEdit> Data;

	//add type check
	template<class T>
	bool draw(T& vec) {
		return draw(reinterpret_cast<float*>(&vec));
	}
private:
	bool draw(float* vec) {
		ImGui::Columns(2);
		ImGui::SetColumnWidth(1, 200.0f);

		ImGui::Text(mName.c_str());
		ImGui::NextColumn();

		ImGui::SetNextItemWidth(mWidth);

		bool res = false;
		auto _nameId = "##" + mName;
		if (mIsColorMode) {
			if (drawColorN(_nameId, mVecSize, vec)) {
				res = true;
			}
		}
		else {
			if (drawFloatN(_nameId, mVecSize, vec)) {
				res = true;
			}
		}
		if (mMode == MODE::COMBINE) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(50);
			if (ImGui::SmallButton(mIsColorMode ? mButtonLblColor.c_str() : mButtonLblVec.c_str())) {
				mIsColorMode = !mIsColorMode;
			}
		}

		ImGui::NextColumn();
		ImGui::Columns(1);
		return res;
	}
};

class ScalarEdit {
	std::string mName;
public:

	inline static std::unordered_map<std::string, ScalarEdit> Data;
	enum class TYPE {
		INT, FLOAT, BOOL
	};
	TYPE mType;
	ScalarEdit(std::string_view name, TYPE type): mType(type), mName(name) {
		
	}
	template<class T>
	bool draw(T& val) {
		bool res = false;
		ImGui::Columns(2);
		ImGui::SetColumnWidth(1, 200.0f);

		ImGui::Text(mName.c_str());
		ImGui::NextColumn();

		ImGui::SetNextItemWidth(200.0f);

		const auto _nameId = "##" + mName;

		res = draw(_nameId, val);

		ImGui::NextColumn();
		ImGui::Columns(1);
		return res;
	}
private:
	template<class T>
	bool draw(const std::string& _nameId, T& val) {
		return false;
	}
	template<>
	bool draw<int>(const std::string& _nameId, int& val) {
		return ImGui::DragInt(_nameId.c_str(), &val);
	}
	template<>
	bool draw<float>(const std::string& _nameId, float& val) {
		return ImGui::DragFloat(_nameId.c_str(), &val, 0.1f);
	}
	template<>
	bool draw<bool>(const std::string& _nameId, bool& val) {
		return ImGui::Checkbox(_nameId.c_str(), &val);
	}
};

bool isEngineUniform(const std::string& uniformName) {
	return uniformName.rfind("EngineUBO", 0) == 0 || uniformName.rfind("engine_", 0) == 0
		|| uniformName.rfind("Engine", 0) == 0;
}
#ifdef OPENGL_BACKEND
void DebugRender::drawMaterialWidget(RENDER::MaterialGl* material) {
	ImGui::Begin("Material Editor");

	auto shader = material->getShader();
	const auto& refl = shader->getUniformsInfo();

	for (auto& [name, data]: refl) {
		if (isEngineUniform(name)) continue;
		if (data.type == RENDER::UniformInform::TYPE::UNIFORM) {
			switch (data.members[0].type) {
			case RENDER::UNIFORM_TYPE::MAT4: break;
			case RENDER::UNIFORM_TYPE::MAT3: break;
			case RENDER::UNIFORM_TYPE::VEC4: {
				if (!CombineVecEdit::Data.count(name)) {
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 4) });
				}
				auto val = std::get<MATHGL::Vector4>(material->get(name));
				if (CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC3: {
				if (!CombineVecEdit::Data.count(name)) {
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 3) });
				}
				auto val = std::get<MATHGL::Vector3>(material->get(name));
				if (CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::VEC2: {
				if (!CombineVecEdit::Data.count(name)) {
					CombineVecEdit::Data.insert({ name, CombineVecEdit(name, 2) });
				}
				auto val = std::get<MATHGL::Vector2f>(material->get(name));
				if (CombineVecEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::INT: {
				if (!ScalarEdit::Data.count(name)) {
					ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::INT) });
				}
				auto val = std::get<int>(material->mUniformData.at(name));
				if (ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::FLOAT: {
				if (!ScalarEdit::Data.count(name)) {
					ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::FLOAT) });
				}
				auto val = std::get<float>(material->mUniformData.at(name));
				if (ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::BOOL: {
				if (!ScalarEdit::Data.count(name)) {
					ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::BOOL) });
				}
				auto val = std::get<bool>(material->mUniformData.at(name));
				if (ScalarEdit::Data.at(name).draw(val)) {
					material->set(name, val);
				}
				break;
			}
			case RENDER::UNIFORM_TYPE::SAMPLER_2D: {
				auto val = std::get<std::shared_ptr<RENDER::TextureGl>>(material->mUniformData.at(name));
				if (val) {
					//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
					ImGui::Image((void*)val->id, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
				}
				//ImGui::GetWindowDrawList()->AddImage(
				//	(void*)val->getId(),
				//	ImVec2(ImGui::GetCursorScreenPos()),
				//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
				//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));
				break;
			}
			case RENDER::UNIFORM_TYPE::SAMPLER_3D: break;
			case RENDER::UNIFORM_TYPE::SAMPLER_CUBE: break;
			default: break;
			}
		}
		else if (data.type == RENDER::UniformInform::TYPE::UNIFORM_BUFFER) {
			for (const auto& mamber : data.members) {
				auto _name = mamber.name;
				switch (mamber.type) {
				case RENDER::UNIFORM_TYPE::MAT4: break;
				case RENDER::UNIFORM_TYPE::MAT3: break;
				case RENDER::UNIFORM_TYPE::VEC4: {
					if (!CombineVecEdit::Data.count(_name)) {
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 4) });
					}
					auto val = std::get<MATHGL::Vector4>(material->get(name, mamber.name));
					if (CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC3: {
					if (!CombineVecEdit::Data.count(_name)) {
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 3) });
					}
					auto val = std::get<MATHGL::Vector3>(material->get(name, mamber.name));
					if (CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::VEC2: {
					if (!CombineVecEdit::Data.count(_name)) {
						CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 2) });
					}
					auto val = std::get<MATHGL::Vector2f>(material->get(name, mamber.name));
					if (CombineVecEdit::Data.at(_name).draw(val)) {
						material->set(name, mamber.name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::INT: {
					if (!ScalarEdit::Data.count(name)) {
						ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::INT) });
					}
					auto val = std::get<int>(material->mUniformData.at(name));
					if (ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::FLOAT: {
					if (!ScalarEdit::Data.count(name)) {
						ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::FLOAT) });
					}
					auto val = std::get<float>(material->mUniformData.at(name));
					if (ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::BOOL: {
					if (!ScalarEdit::Data.count(name)) {
						ScalarEdit::Data.insert({ name, ScalarEdit(name, ScalarEdit::TYPE::BOOL) });
					}
					auto val = std::get<bool>(material->mUniformData.at(name));
					if (ScalarEdit::Data.at(name).draw(val)) {
						material->set(name, val);
					}
					break;
				}
				case RENDER::UNIFORM_TYPE::SAMPLER_2D: break;
				case RENDER::UNIFORM_TYPE::SAMPLER_3D: break;
				case RENDER::UNIFORM_TYPE::SAMPLER_CUBE: break;
				default: break;
				}
			}
		}
	}
	ImGui::End();
}
#endif

void drawComponent(ECS::TransformComponent* component) {
	if (ImGui::CollapsingHeader(component->getName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		{
			auto val = component->getTransform().getLocalPosition();
			std::string _name = "Position";
			if (!CombineVecEdit::Data.count(_name)) {
				CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 3, CombineVecEdit::MODE::POS) });
			}
			if (CombineVecEdit::Data.at(_name).draw(val)) {
				component->getTransform().setLocalPosition(val);
			}
		}
		{
			auto val = component->getTransform().getLocalScale();
			std::string _name = "Scale";
			if (!CombineVecEdit::Data.count(_name)) {
				CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 3, CombineVecEdit::MODE::POS) });
			}
			if (CombineVecEdit::Data.at(_name).draw(val)) {
				component->getTransform().setLocalScale(val);
			}
		}
		{
			auto val = MATHGL::Quaternion::ToEulerAngles(component->getTransform().getLocalRotation());
			val = { TO_DEGREES(val.x), TO_DEGREES(val.y), TO_DEGREES(val.z) };
			std::string _name = "Rotate";
			if (!CombineVecEdit::Data.count(_name)) {
				CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 3, CombineVecEdit::MODE::POS) });
			}
			if (CombineVecEdit::Data.at(_name).draw(val)) {
				//val = { TO_RADIANS(val.x), TO_RADIANS(val.y), TO_RADIANS(val.z) };
				component->getTransform().setLocalRotation(MATHGL::Quaternion(val));
			}
		}

		if (component->getTransform().isAnchorPivotMode()) {
			{
				auto val = component->getTransform().getLocalAnchor();
				std::string _name = "Anchor";
				if (!CombineVecEdit::Data.count(_name)) {
					CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 2) });
				}
				if (CombineVecEdit::Data.at(_name).draw(val)) {
					component->getTransform().setLocalAnchor(val);
				}
			}
			{
				auto val = component->getTransform().getLocalPivot();
				std::string _name = "Pivot";
				if (!CombineVecEdit::Data.count(_name)) {
					CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 2) });
				}
				if (CombineVecEdit::Data.at(_name).draw(val)) {
					component->getTransform().setLocalPivot(val);
				}
			}
			{
				auto val = component->getTransform().getLocalSize();
				std::string _name = "Size";
				if (!CombineVecEdit::Data.count(_name)) {
					CombineVecEdit::Data.insert({ _name, CombineVecEdit(_name, 2) });
				}
				if (CombineVecEdit::Data.at(_name).draw(val)) {
					component->getTransform().setLocalSize(val);
				}
			}
			
		}
	}
}

void DebugRender::drawComponentInspector() {
	ImGui::Begin("Component Inspector");

	
	if (selectObj) {
		auto components = ECS::ComponentManager::getInstance()->getComponents(selectObj->getID());
		for (auto component : components) {
			if (component->getName() == "Transform") {
				drawComponent(reinterpret_cast<ECS::TransformComponent*>(component.getPtr().get()));
			}
		}
	}

	ImGui::End();

}

void DebugRender::drawTextureWatcher() {
#ifdef OPENGL_BACKEND
	ImGui::Begin("Texture Watcher");

	auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
	auto _renderer = reinterpret_cast<RENDER::GameRendererGl*>(&renderer);


	const char* items[] = { "Before Post Processing", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
	static int item_current = 0;
	ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));
	{
		if (item_current == 0) {
			auto val = _renderer->mDeferredTexture;
			if (val) {
				auto winSize = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
				ImVec2 imWinSize = ImGui::GetWindowSize();

				float w = 0.0f;
				float h = 0.0f;
				
				if (imWinSize.x < imWinSize.y) {
					w = imWinSize.x;
					h = (winSize.y * imWinSize.x) / winSize.x;
				}
				else
				{
					w = (imWinSize.y * winSize.x) / winSize.y;
					h = imWinSize.y;
				}

				ImGui::Image((void*)val->id, ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
			}
		}
	}

	ImGui::End();
#endif
}

void drawMainWindow()
{

	static bool isOpen = true;
	//static const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
	//ImGuiID dockSpace = ImGui::GetID("MainWindowDockspace");
	//ImGui::DockSpace(dockSpace, ImVec2(0.0f, 0.0f), dockspaceFlags);
	{//menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Examples"))
			{
				ImGui::MenuItem("Main menu bar", NULL, &[](){});
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}

#ifdef IMGUI_HAS_VIEWPORT
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	auto h = ImGui::GetFrameHeight();
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + h), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - h), ImGuiCond_Always);
	ImGui::SetNextWindowViewport(viewport->ID);
#else 
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif



	ImGui::Begin("##MainWin", &isOpen,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
	
	ImGui::End();
}

#include "../3rd/imgui/ImCurveEdit.h"
#include "../3rd/imgui/ImSequencer.h"

#include "../utils/animation.h"
//#include "../utils/refl.hpp"

template<class T>
void addWrappersForComponent(ANIMATION::Animation& anim, T& component) {
	//static_assert()
}

template<>
void addWrappersForComponent(ANIMATION::Animation& anim, ECS::TransformComponent& component) {
	using ComponentType = ECS::TransformComponent;

	auto id = component.getObject().getID();
	std::string prefix = std::to_string(static_cast<int>(id));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "posX",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			component->setLocalPosition({ std::get<float>(val), pos.y, pos.z });
		},
		[id]() { 
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			return pos.x;
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "posY",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			component->setLocalPosition({ pos.x, std::get<float>(val), pos.z });
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			return pos.y;
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "posZ",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			component->setLocalPosition({ pos.x, pos.y, std::get<float>(val) });
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalPosition();
			return pos.z;
		}
	));

	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "rotX",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ std::get<float>(val), TO_DEGREES(rot.y), TO_DEGREES(rot.z) };
			component->setLocalRotation(MATHGL::Quaternion({ rotVec }));
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			return TO_DEGREES(rot.x);
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "rotY",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ TO_DEGREES(rot.x), std::get<float>(val), TO_DEGREES(rot.z) };
			component->setLocalRotation(MATHGL::Quaternion({ rotVec }));
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			return TO_DEGREES(rot.y);
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "rotY",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			MATHGL::Vector3 rotVec = MATHGL::Vector3{ TO_DEGREES(rot.x), TO_DEGREES(rot.y), std::get<float>(val) };
			component->setLocalRotation(MATHGL::Quaternion({ rotVec }));
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto rot = MATHGL::Quaternion::ToEulerAngles(component->getLocalRotation());
			return TO_DEGREES(rot.z);
		}
	));

	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "scaleX",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			component->setLocalScale({ std::get<float>(val), pos.y, pos.z });
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			return pos.x;
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "scaleY",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			component->setLocalScale({ pos.x, std::get<float>(val), pos.z });
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			return pos.y;
		}
	));
	anim.addProperty(ANIMATION::AnimationProperty(
		prefix + "scaleZ",
		[id](ANIMATION::PropType val) {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			component->setLocalScale({ pos.x, pos.y, std::get<float>(val) });
		},
		[id]() {
			auto component = ECS::ComponentManager::getInstance()->getComponent<ComponentType>(id);
			auto pos = component->getLocalScale();
			return pos.z;
		}
	));

	
}

using ComponentName = std::string;
using PromName = std::string;
using PropValuse = std::variant<float, int, bool, std::string>;
std::unordered_map<ComponentName, std::map<PromName, PropValuse>> componentsProperties;


struct MySequence : public ImSequencer::SequenceInterface {
	std::unique_ptr<KUMA::ANIMATION::Animation> animation;

	virtual int GetFrameMin() const {
		return mFrameMin;
	}
	virtual int GetFrameMax() const {
		return mFrameMax;
	}
	virtual int GetItemCount() const { return (int)myItems.size(); }

	virtual int GetItemTypeCount() const { return SequencerItemTypeNames.size(); }
	virtual const std::string GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex].name + ":" + SequencerItemTypeNames[typeIndex].componentName; }
	virtual const char* GetItemLabel(int index) const
	{
		static char tmps[512];
		snprintf(tmps, 512, "[%02d]", index);
		return tmps;
	}

	virtual void Get(int index, std::vector<std::pair<int, int>>& intervals, int* type, unsigned int* color) override
	{
		MySequenceItem& item = myItems[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		intervals = item.intervals;
		if (type)
			*type = item.mType;
	}

	virtual void RemoveInterval(int eId, int iId) override {
		if (eId < 0 || iId < 0) {
			return;
		}

		std::map<std::string, ANIMATION::PropType> prop;
		for (auto& e : myItems[eId].intervalsValues[iId]) {
			prop[std::to_string(static_cast<int>(myItems[eId].info.id)) + e.first] = e.second;
		}
		animation->delKeyFrameProp(myItems[eId].intervals[iId].first, prop);

		myItems[eId].intervals.erase(myItems[eId].intervals.begin() + iId);
		myItems[eId].intervalsValues.erase(myItems[eId].intervalsValues.begin() + iId);
	}

	virtual void AddInterval(int eId, int frame) override {
		if (eId < 0 || frame < 0) {
			return;
		}
		myItems[eId].intervals.push_back(std::make_pair(frame, frame));
		myItems[eId].intervalsValues.push_back(componentsProperties[myItems[eId].info.componentName]);
		

		std::map<std::string, ANIMATION::PropType> prop;
		for (auto& e : myItems[eId].intervalsValues.back()) {
			prop[std::to_string(static_cast<int>(myItems[eId].info.id)) + e.first] = e.second;
		}
		animation->delKeyFrameProp(myItems[eId].intervals.back().first, prop);
	}

	virtual void UpdateFPS(unsigned int fps) override {
		animation->setFPS(fps);
	}
	virtual void UpdateMaxFrame(unsigned int mframe) override {
		animation->setFrameCount(mframe);
	}

	virtual void SetIsLooped(bool b) override {
		animation->setLooped(b);
	}


	virtual void Set(int index, std::vector<std::pair<int, int>>& intervals) override
	{
		MySequenceItem& item = myItems[index];
		item.intervals = intervals;
	}

	virtual void Add(int type) {
		auto& data = SequencerItemTypeNames[type];

		static int lineId = 0;
		myItems.push_back(MySequenceItem{ data, lineId++, {}, false, {} });

		if (data.componentName == "Transform") {
			addWrappersForComponent(*animation, selectObj->getComponent<ECS::TransformComponent>().value().get());
		}
	};

	//TODO: del from anim
	virtual void Del(int index) { myItems.erase(myItems.begin() + index); }

	virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

	virtual size_t GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

	MySequence() : mFrameMin(0), mFrameMax(0)
	{
		animation = std::make_unique<ANIMATION::Animation>(mFrameMax, FPS, false);

		componentsProperties["Transform"] = {
			{"posX", 0.0f},
			{"posY", 0.0f},
			{"posZ", 0.0f},
			{"rotX", 0.0f},
			{"rotY", 0.0f},
			{"rotZ", 0.0f},
			{"scaleX", 0.0f},
			{"scaleY", 0.0f},
			{"scaleZ", 0.0f}
		};
	}
	int FPS = 30;
	int mFrameMin, mFrameMax;
	struct MySequenceItem
	{
		AnimationLineInfo info;
		int mType;
		std::vector<std::pair<int, int>> intervals;
		bool mExpanded;
		std::vector<std::map<PromName, PropValuse>> intervalsValues;
	};
	std::vector<MySequenceItem> myItems;
	//RampEdit rampEdit;

	virtual void DoubleClick(int index) {
		//if (myItems[index].mExpanded)
		//{
		//	myItems[index].mExpanded = false;
		//	return;
		//}
		//for (auto& item : myItems)
		//	item.mExpanded = false;
		//myItems[index].mExpanded = !myItems[index].mExpanded;
	}

	void applyFrame(MySequenceItem& item, int intervalId) {
		std::map<std::string, ANIMATION::PropType> prop;
		for (auto& e : item.intervalsValues[intervalId]) {
			prop[std::to_string(static_cast<int>(item.info.id)) + e.first] = e.second;
		}
		animation->addKeyFrameMerge(item.intervals[intervalId].first, prop);
	}

	virtual void UpdateFrame(int id, int frameId, int o, int n) override
	{
		auto& item = myItems[id];
		updateFrame(item, frameId, o, n);
	};

	void updateFrame(MySequenceItem& item, int frameId, int oldId, int newId) {
		std::map<std::string, ANIMATION::PropType> prop;
		for (auto& e : item.intervalsValues[frameId]) {
			prop[std::to_string(static_cast<int>(item.info.id)) + e.first] = e.second;
		}

		animation->delKeyFrameProp(oldId, prop);
		animation->addKeyFrameMerge(newId, prop);
	}

	virtual void Update()
	{
		animation->update(TIME::Timer::GetInstance().getDeltaTime().count() * 1000.0f);
	}

	virtual void FrameDraw(int index, int intervalId, ImDrawList* draw_list)
	{
		if (index < 0 || intervalId < 0) {
			return;
		}
		//static const char* labels[] = { "Translation", "Rotation" , "Scale" };
		//ImGui::SameLine();
		//ImGui::SetCursorScreenPos(rc.Min);
		ImGui::Begin("##EditFrame");
		//static float[3];
		auto& item = myItems[index];
		auto& itemInterval = item.intervalsValues[intervalId];

		for (auto& e : itemInterval) {
			std::string name = e.first;
			std::visit([this, name, &e, &item, intervalId](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, int>) {
					if (ImGui::DragInt(name.c_str(), &arg)) {
						e.second = arg;
						applyFrame(item, intervalId);
					}
				}
				else if constexpr (std::is_same_v<T, float>) {
					if (ImGui::DragFloat(name.c_str(), &arg)) {
						e.second = arg;
						applyFrame(item, intervalId);
					}
				}
				else if constexpr (std::is_same_v<T, bool>) {
					if (ImGui::Checkbox(name.c_str(), &arg)) {
						e.second = arg;
						applyFrame(item, intervalId);
					}
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					if (ImGui::InputText(name.c_str(), &arg)) {
						e.second = arg;
						applyFrame(item, intervalId);
					}
				}
			}, e.second);
		}

		//for (auto& e : itemInterval.second) {
		//	std::string name = "End " + e.first;
		//	std::visit([name, &e](auto&& arg) {
		//		using T = std::decay_t<decltype(arg)>;
		//		if constexpr (std::is_same_v<T, int>) {
		//			if (ImGui::DragInt(name.c_str(), &arg)) {
		//				e.second = arg;
		//			}
		//		}
		//		else if constexpr (std::is_same_v<T, float>) {
		//			if (ImGui::DragFloat(name.c_str(), &arg)) {
		//				e.second = arg;
		//			}
		//		}
		//		else if constexpr (std::is_same_v<T, bool>) {
		//			if (ImGui::Checkbox(name.c_str(), &arg)) {
		//				e.second = arg;
		//			}
		//		}
		//		else if constexpr (std::is_same_v<T, std::string>) {
		//			if (ImGui::InputText(name.c_str(), &arg)) {
		//				e.second = arg;
		//			}
		//		}
		//	}, e.second);
		//}
		//ImGui::DragFloat3("Transform", e);
		//ImGui::DragFloat3("Rotation", e);
		//ImGui::DragFloat3("Scale", e);
		ImGui::End();
		//rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		//rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		//draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
		//for (int i = 0; i < 3; i++)
		//{
		//	ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
		//	ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
		//	draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
		//	if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
		//		rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
		//}
		//draw_list->PopClipRect();
		//
		//ImGui::SetCursorScreenPos(rc.Min);
		//
		//ImCurveEdit::Edit(rampEdit, { rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y }, 137 + index, &clippingRect);
	}

	virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
	{
		//rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		//rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		//draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
		//for (int i = 0; i < 3; i++)
		//{
		//	for (int j = 0; j < rampEdit.mPointCount[i]; j++)
		//	{
		//		float p = rampEdit.mPts[i][j].x;
		//		if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
		//			continue;
		//		float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
		//		float x = ImLerp(rc.Min.x, rc.Max.x, r);
		//		draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
		//	}
		//}
		//draw_list->PopClipRect();
	}
};

MySequence mySequence;

void DebugRender::draw(CORE_SYSTEM::Core& core) {

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_NewFrame();
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplVulkan_NewFrame();
	}
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMainWindow();

	{//Debug
		ImGui::Begin("Render pipeline");

		/*static bool isBloom = true;
		ImGui::Checkbox("Bloom", &isBloom);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::BLOOM, isBloom);

		static bool isGodRay = true;
		ImGui::Checkbox("God ray", &isGodRay);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::GOOD_RAYS, isGodRay);

		static bool isMotionBlur = true;
		ImGui::Checkbox("Motion blur", &isMotionBlur);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::MOTION_BLUR, isMotionBlur);

		static bool isFXAA = true;
		ImGui::Checkbox("FXAA", &isFXAA);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::FXAA, isFXAA);

		static bool isHDR = true;
		ImGui::Checkbox("HDR", &isHDR);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::HDR, isHDR);

		static bool isVIGNETTE = true;
		ImGui::Checkbox("VIGNETTE", &isVIGNETTE);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::VIGNETTE, isVIGNETTE);

		static bool isCOLOR_GRADING = true;
		ImGui::Checkbox("COLOR_GRADING", &isCOLOR_GRADING);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::COLOR_GRADING, isCOLOR_GRADING);

		static bool isCHROMATIC_ABBERATION = true;
		ImGui::Checkbox("CHROMATIC_ABBERATION", &isCHROMATIC_ABBERATION);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::CHROMATIC_ABBERATION, isCHROMATIC_ABBERATION);

		static bool isVOLUMETRIC_LIGHT = true;
		ImGui::Checkbox("VOLUMETRIC_LIGHT", &isVOLUMETRIC_LIGHT);
		core.renderer->setPostProcessing(RENDER::Renderer::PostProcessing::VOLUMETRIC_LIGHT, isVOLUMETRIC_LIGHT);

		//func(core.renderer->pipeline.vignette);
		//constexpr auto info = serde::type_info<RENDER::Vignette>;
		buildWidget(core.renderer->pipeline.vignette);

		if (selectObjGui) {
			{
				ImGui::DragFloat("PivotX", &selectObjGui->transform->pivot.x, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("PivotY", &selectObjGui->transform->pivot.y, 0.1f, 0.0f, 1.0f);
			}
			{
				ImGui::DragFloat("AnchorX", &selectObjGui->transform->anchor.x, 0.1f, 0.0f, 1.0f);
				ImGui::DragFloat("AnchorY", &selectObjGui->transform->anchor.y, 0.1f, 0.0f, 1.0f);
			}
			{
				ImGui::DragFloat("PoxX", &selectObjGui->transform->position.x, 1.0f);;
				ImGui::DragFloat("PosY", &selectObjGui->transform->position.y, 1.0f);
			}
			{
				ImGui::DragFloat("ScaleX", &selectObjGui->transform->scale.x, 1.0f);
				ImGui::DragFloat("ScaleY", &selectObjGui->transform->scale.y, 1.0f);
			}
			{
				ImGui::DragFloat("RotX", &selectObjGui->transform->rotation.x, 1.0f);
				ImGui::DragFloat("RotY", &selectObjGui->transform->rotation.z, 1.0f);
			}
		}*/

		/* {
			static float a = 0.00f;
			ImGui::DragFloat("PivotX", &a, 0.1f, 0.0f, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("PivotY", &b, 0.1f, 0.0f, 1.0f);
			core.renderer->f.pivot = MATHGL::Vector2f(a, b);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("AnchorX", &a, 0.1f, 0.0f, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("AnchorY", &b, 0.1f, 0.0f, 1.0f);
			core.renderer->f.anchor = MATHGL::Vector2f(a, b);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("PoxX", &a, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("PosY", &b, 1.0f);
			core.renderer->f.position = MATHGL::Vector3(a, b, 0.0f);
		}
		{
			static float a = 1.00f;
			ImGui::DragFloat("ScaleX", &a, 1.0f);
			static float b = 1.00f;
			ImGui::DragFloat("ScaleY", &b, 1.0f);
			core.renderer->f.scale = MATHGL::Vector3(a, b, 1.0f);
		}
		{
			static float a = 0.00f;
			ImGui::DragFloat("RotX", &a, 1.0f);
			static float b = 0.00f;
			ImGui::DragFloat("RotY", &b, 1.0f);
			core.renderer->f.rotation = MATHGL::Vector3(a, 0.0f, b);
		}*/

		auto& im = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>();
		if (im.isGamepadExist(0)) {
			for (auto e : im.getGamepad(0).buttons) {
				ImGui::LabelText(("Btn" + std::to_string(static_cast<int>(e.first))).c_str(), std::to_string(e.second).c_str());
			}
			ImGui::LabelText("LStick", (std::to_string(im.getGamepad(0).leftSticX) + " " + std::to_string(im.getGamepad(0).leftSticY)).c_str());
			ImGui::LabelText("RStick", (std::to_string(im.getGamepad(0).rightSticX) + " " + std::to_string(im.getGamepad(0).rightSticX)).c_str());
			ImGui::LabelText("Triggers", (std::to_string(im.getGamepad(0).leftTrigger) + " " + std::to_string(im.getGamepad(0).rightTrigger)).c_str());
		}

		auto boxMaterial = ECS::ComponentManager::getInstance()->getComponent<ECS::MaterialRenderer>(ECS::Entity(3));
#ifdef OPENGL_BACKEND
		if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
			drawMaterialWidget(reinterpret_cast<RENDER::MaterialGl*>(boxMaterial.getPtr()->GetMaterialAtIndex(0).get()));
		}
#endif

		drawComponentInspector();
		drawTextureWatcher();
		ImGui::End();
	}
	drawWindowWidget(core);
	{
		drawNodeTree(core);
		//drawNodeTreeGui(core);
	}
	{
		//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		//ImGui::Begin("Scene Window");
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)core.renderer->getResultTexture().getId(),
		//	ImVec2(ImGui::GetCursorScreenPos()),
		//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
		//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));
		//ImGui::End();
	}
	{
		//auto size = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().getSize();
		//ImGui::Begin("Scene Window Light");
		//ImVec2 pos = ImGui::GetCursorScreenPos();
		//ImGui::GetWindowDrawList()->AddImage(
		//	(void*)core.renderer->getResultTexture2().getId(),
		//	ImVec2(ImGui::GetCursorScreenPos()),
		//	ImVec2(ImGui::GetCursorScreenPos().x + size.x / 2,
		//		ImGui::GetCursorScreenPos().y + size.y / 2), ImVec2(0, 1), ImVec2(1, 0));

		{
			//static float x = -20.0f;
			//static float y = 40.0f;
			//static float z = 10.0f;
			//if (ImGui::DragFloat("LightX", &x, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
			//if (ImGui::DragFloat("LightY", &y, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
			//if (ImGui::DragFloat("LightZ", &z, 1.0f, -100.0f, 100.0f)) {
			//	core.renderer->setLightPos(x, y, z);
			//}
		}

		//ImGui::End();
	}
	//ImGui::SFML::Render(*window);

	{
		fileBrowser->draw();
	}

	{
		ImGui::Begin("Seq");
		static int selectedEntry = -1;
		static int firstFrame = 0;
		static bool expanded = true;
		static int currentFrame = 100;

		static bool isInit = false;
		if (!isInit)
		{
			isInit = true;
			mySequence.mFrameMin = 0;
			mySequence.mFrameMax = 1000;
			//mySequence.myItems.push_back(MySequence::MySequenceItem{ 0, 10, 30, false });
			//mySequence.myItems.push_back(MySequence::MySequenceItem{ 1, 20, 30, true });
			//mySequence.myItems.push_back(MySequence::MySequenceItem{ 3, 12, 60, false });
			//mySequence.myItems.push_back(MySequence::MySequenceItem{ 2, 61, 90, false });
			//mySequence.myItems.push_back(MySequence::MySequenceItem{ 4, 90, 99, false });
		}

		ImGui::PushItemWidth(130);
		ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
		ImGui::SameLine();
		ImGui::InputInt("Frame ", &currentFrame);
		ImGui::SameLine();
		if (ImGui::InputInt("Frame Max", &mySequence.mFrameMax)) {
			mySequence.UpdateMaxFrame(mySequence.mFrameMax);
		};
		static bool isLooped = false;
		if (ImGui::Checkbox("Is Looped", &isLooped)) {
			mySequence.SetIsLooped(isLooped);
		};
		if (ImGui::Button("Play")) {
			mySequence.animation->stop();
			mySequence.animation->play();
		};
		ImGui::PopItemWidth();
		ImSequencer::Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
		// add a UI to edit that particular item
		if (selectedEntry != -1)
		{
			const MySequence::MySequenceItem& item = mySequence.myItems[selectedEntry];
			ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
			// switch (type) ....
		}
		ImGui::End();
	}

	ImGui::Render();

	//

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
#endif
#ifdef VULKAN_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
#endif
	
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
	//window->popGLStates();
}