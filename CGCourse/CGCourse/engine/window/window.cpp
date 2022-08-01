#include "window.h"

#include <fstream>

#include "../config.h"
#include "../utils/gamepad/gamepadManager.h"
//imgui
#include "../core/core.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

import logger;

using namespace KUMA;
using namespace KUMA::WINDOW_SYSTEM;

static void glfwErrorCallback(int error, const char* description) {
	LOG_ERROR("Glfw Error: " + std::to_string(error) + " " + description);
}

Window::Window(const WindowSettings& p_windowSettings): windowSettings(p_windowSettings) {
	nlohmann::json j;
	std::ifstream i(Config::ENGINE_ASSETS_PATH + "Configs\\" + "app.json");
	i >> j;
	onDeserialize(j);
	create();
}

Window::~Window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

bool Window::isClosed() const {
	return m_isClosed;
}

void Window::setSize(unsigned int w, unsigned int h) {
	glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	glfwSetWindowPos(window.get(), x, y);
}

void Window::hide() const {
	glfwHideWindow(window.get());
}

void Window::show() const {
	glfwShowWindow(window.get());
}

void Window::focus() const {
	glfwFocusWindow(window.get());
}

bool Window::hasFocus() const {
	return glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void Window::setFullscreen(bool val) {
	isFullscreen = val;
	glfwSetWindowMonitor(
		window.get(),
		val ? glfwGetPrimaryMonitor() : nullptr,
		position.first,
		position.second,
		size.first,
		size.second,
		refreshRate
	);
}

void Window::toggleFullscreen() {
	setFullscreen(!isFullscreen);
}

bool Window::getIsFullscreen() const {
	return isFullscreen;
}

void Window::setTitle(const std::string& _title) {
	title = _title;
	glfwSetWindowTitle(window.get(), title.c_str());
}

std::string Window::getTitle() const {
	return title;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyPressedEvent.run(key);
	}
	else if (action == GLFW_RELEASE) {
		keyReleasedEvent.run(key);
	}
}

void Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouseButtonPressedEvent.run(button);
	}
	else if (action == GLFW_RELEASE) {
		mouseButtonReleasedEvent.run(button);
	}
}

void Window::DestroyglfwWin::operator()(GLFWwindow* ptr) const {
	glfwDestroyWindow(ptr);
}

void Window::create() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, windowSettings.majorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, windowSettings.minorVersion);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = std::unique_ptr<GLFWwindow, DestroyglfwWin>(glfwCreateWindow(size.first, size.second, title.c_str(), NULL, NULL));


	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); //vsync
	//auto settings = window->getSettings();
	//settings.depthBits = windowSettings.depthBits;
	//settings.stencilBits = windowSettings.stencilBits;
	//settings.antialiasingLevel = windowSettings.antialiasingLevel;
	//settings.majorVersion = windowSettings.majorVersion;
	//settings.minorVersion = windowSettings.minorVersion;
	glfwWindowHint(GLFW_REFRESH_RATE, 60);


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

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);


	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
	else {
		int x, y;
		glfwGetWindowPos(window.get(), &x, &y);
		position.first = x;
		position.second = y;
	}

	INPUT::GamepadManager::Instance();

	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().keyCallback(window, key, scancode, action, mods);
	});
	
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().mouseCallback(window, button, action, mods);
	});
	glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
		RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>().m_isClosed = true;
	});
}

void Window::update() {
	INPUT::GamepadManager::Instance().update([this](INPUT::Gamepad::GamepadData& data) {
		gamepadEvent.run(data);
	});
}

int uniqueNodeId = 0;
std::shared_ptr<KUMA::ECS::Object> selectObj;
std::shared_ptr<KUMA::GUI::GuiObject> selectObjGui;

std::shared_ptr<KUMA::ECS::Object> recursiveDraw(KUMA::SCENE_SYSTEM::Scene& activeScene, std::shared_ptr<KUMA::ECS::Object> parentEntity) {
	std::shared_ptr<KUMA::ECS::Object> selectedNode;
	
	std::vector<std::shared_ptr<KUMA::ECS::Object>> nodeList;

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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->getName().c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5.0f, 5.0f});
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {
				
			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
		ImGui::InvisibleButton("__NODE_ORDER_SET__", {-1, 5});
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
			}
		}
		ImGui::End();
	}
}


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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
		bool nodeIsOpen = ImGui::TreeNodeBehavior(ImGui::GetCurrentWindow()->GetID(node->name.c_str()), nodeFlags, name.c_str());
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{5.0f, 5.0f});
		if (ImGui::BeginPopupContextItem("__SCENE_TREE_CONTEXTMENU__")) {
			if (ImGui::MenuItem("Create new")) {

			}
			if (ImGui::MenuItem("Delete")) {

			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.0f, 10.0f});
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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
		ImGui::InvisibleButton("__NODE_ORDER_SET__", {-1, 5});
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
void drawNodeTreeGui(KUMA::CORE_SYSTEM::Core& core) {
	static bool isobjTreeOpen = true;
	if (core.sceneManager->hasCurrentScene()) {
		auto& scene = core.sceneManager->getCurrentScene();
		if (ImGui::Begin("Scene Hierarchy", &isobjTreeOpen)) {
			uniqueNodeId = 0;
			auto _selectNode = recursiveDrawGui(scene, nullptr);
			if (_selectNode) {
				selectObjGui = _selectNode;
			}
		}
		ImGui::End();
	}
}

void Window::drawDebug(CORE_SYSTEM::Core& core) {
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//window->pushGLStates();
	{//Debug
		//ImGui::SFML::Update(*window, deltaClock.restart());

		ImGui::Begin("Render pipeline");

		static bool isBloom = true;
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
		}

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
		ImGui::End();
	}
	{
		drawNodeTree(core);
		drawNodeTreeGui(core);
	}
	{
		ImGui::Begin("Scene Window");
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddImage(
			(void*)core.renderer->getResultTexture().getId(),
			ImVec2(ImGui::GetCursorScreenPos()),
			ImVec2(ImGui::GetCursorScreenPos().x + size.first / 2,
				ImGui::GetCursorScreenPos().y + size.second / 2), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}
	//ImGui::SFML::Render(*window);
	
	ImGui::Render();

	//


	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
	//window->popGLStates();
	
}

void Window::draw() {
	if (!glfwWindowShouldClose(window.get())) {
		glfwSwapBuffers(window.get());
		//glfwPollEvents();
	}
}

GLFWwindow& Window::getContext() const {
	return *window;
}

void Window::onSerialize(nlohmann::json& j) {
	j["window"]["depthBits"] = windowSettings.depthBits;
	j["window"]["stencilBits"] = windowSettings.stencilBits;
	j["window"]["antialiasingLevel"] = windowSettings.antialiasingLevel;
	j["window"]["majorVersion"] = windowSettings.majorVersion;
	j["window"]["minorVersion"] = windowSettings.minorVersion;
	j["window"]["isFillscreen"] = windowSettings.isFullscreen;
	j["window"]["appName"] = windowSettings.appName;
}

void Window::onDeserialize(nlohmann::json& j) {
	windowSettings.depthBits = j["window"]["depthBits"];
	windowSettings.stencilBits = j["window"]["stencilBits"];
	windowSettings.antialiasingLevel = j["window"]["antialiasingLevel"];
	windowSettings.majorVersion = j["window"]["majorVersion"];
	windowSettings.minorVersion = j["window"]["minorVersion"];
	windowSettings.isFullscreen = j["window"]["isFillscreen"];
	windowSettings.appName = j["window"]["appName"];
}

MATHGL::Vector2i Window::getMousePos() {
	double x, y;
	glfwGetCursorPos(window.get(), &x, &y);
	return MATHGL::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
MATHGL::Vector2u Window::getSize() const {
	auto width = 0;
	auto height = 0;
	glfwGetWindowSize(window.get(), &width, &height);
	return MATHGL::Vector2u(width, height);
}