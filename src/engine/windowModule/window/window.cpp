#include "window.h"

#include "utilsModule/log/loggerDefine.h"

#ifdef USE_SDL
#include <SDL.h>

#include "coreModule/platform.hpp"
#include "windowModule/inputManager/inputManager.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if defined(USE_EDITOR) || defined(USE_CHEATS)
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#endif

using namespace IKIGAI;
using namespace IKIGAI::WINDOW;

const std::map<SDL_GameControllerButton, INPUT::Gamepad::GAMEPAD_BUTTON> ToGamepadButton = {
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A, INPUT::Gamepad::GAMEPAD_BUTTON::btn_a},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B, INPUT::Gamepad::GAMEPAD_BUTTON::btn_b},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X, INPUT::Gamepad::GAMEPAD_BUTTON::btn_x},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y, INPUT::Gamepad::GAMEPAD_BUTTON::btn_y},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK, INPUT::Gamepad::GAMEPAD_BUTTON::btn_leftStick},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK, INPUT::Gamepad::GAMEPAD_BUTTON::btn_rightStick},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK, INPUT::Gamepad::GAMEPAD_BUTTON::btn_back},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START, INPUT::Gamepad::GAMEPAD_BUTTON::btn_start},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER, INPUT::Gamepad::GAMEPAD_BUTTON::btn_lb},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, INPUT::Gamepad::GAMEPAD_BUTTON::btn_rb},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP, INPUT::Gamepad::GAMEPAD_BUTTON::dpad_up},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN, INPUT::Gamepad::GAMEPAD_BUTTON::dpad_down},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT, INPUT::Gamepad::GAMEPAD_BUTTON::dpad_left},
	{SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT, INPUT::Gamepad::GAMEPAD_BUTTON::dpad_right},
};

struct Window::Internal {
	Internal() = default;
	SDL_Window* mWindow = nullptr;
	SDL_GLContext mContext;
	bool mIsFocus = true;
	std::set<SDL_GameController*> mGamepads;

	void addGamepad(SDL_GameController* gp) {
		mGamepads.insert(gp);
	}
	void removeGamepad(SDL_GameController* gp) {
		mGamepads.erase(gp);
	}
};

std::vector<SDL_GameController*> findController() {
	std::vector<SDL_GameController*> res;
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			res.push_back(SDL_GameControllerOpen(i));
		}
	}
	return res;
}

Window::Window(const WindowSettings& p_windowSettings) : mWindowSettings(p_windowSettings), mContext(std::make_unique<Internal>()) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
	create();

	const auto gamepads = findController();
	for (auto gp : gamepads) {
		int id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp));
		std::string name = SDL_GameControllerName(gp);
		gamepadAddEvent.run(INPUT::Gamepad(id, name));
	}

}

Window::~Window() {
	SDL_GL_DeleteContext(mContext->mContext);
	SDL_DestroyWindow(mContext->mWindow);
	SDL_Quit();
}

MATH::Vector2i Window::getMousePos() const {
	MATH::Vector2i res;
	SDL_GetMouseState(&res.x, &res.y);
	return res;
}

void Window::setSize(unsigned width, unsigned height) {

	switch (GetCurrentPlatform()) {
		case Platform::IOS:
		case Platform::ANDROIDOS:
		case Platform::EMSCRIPT: {
			//Can not change window size
			break;
		}
		case Platform::WINDOWS:
		case Platform::MAC: {
			mWindowSettings.size = { width, height };
			SDL_SetWindowSize(mContext->mWindow, width, height);
		}
		default: {
			break;
		}
	}
}

MATH::Vector2u Window::getSize() const
{
	uint32_t displayWidth{ 0 };
	uint32_t displayHeight{ 0 };

#ifdef __EMSCRIPTEN__
	// For Emscripten targets we will invoke some Javascript
	// to find out the dimensions of the canvas in the HTML
	// document. Note that the 'width' and 'height' attributes
	// need to be set on the <canvas /> HTML element, like so:
	// <canvas id="canvas" width="600", height="360"></canvas>
	displayWidth = static_cast<uint32_t>(EM_ASM_INT({
		return document.getElementById('canvas').width;
	}));

	displayHeight = static_cast<uint32_t>(EM_ASM_INT({
		return document.getElementById('canvas').height;
	}));
#else
	switch (GetCurrentPlatform()) {
	case Platform::IOS:
	case Platform::ANDROIDOS: {
		// For mobile platforms we will fetch the full screen size.
		SDL_DisplayMode displayMode;
		SDL_GetDesktopDisplayMode(0, &displayMode);
		displayWidth = static_cast<uint32_t>(displayMode.w);
		displayHeight = static_cast<uint32_t>(displayMode.h);
		break;
	}
	default: {
		// For other platforms we'll just show a fixed size window.
		displayWidth = mWindowSettings.size.x;
		displayHeight = mWindowSettings.size.y;
		break;
	}
	}
#endif

	return MATH::Vector2u(displayWidth, displayHeight);
}

void Window::setPosition(int x, int y) {
	SDL_SetWindowPosition(mContext->mWindow, x, y);
}

MATH::Vector2i Window::getPosition() const {
	MATH::Vector2i res;
	SDL_GetWindowPosition(mContext->mWindow, &res.x, &res.y);
	return res;
}

void Window::setTitle(const std::string& title) {
	SDL_SetWindowTitle(mContext->mWindow, title.c_str());
	mWindowSettings.title = title;
}

std::string Window::getTitle() const {
	return SDL_GetWindowTitle(mContext->mWindow);
}

void Window::setDepthBits(int val) {
	if (mContext->mWindow) return;
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, val);
	mWindowSettings.depthBits = val;
}

int Window::getDepathBits() const {
	int res = 0;
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &res);
	return res;
}

void Window::setStencilBits(int val) {
	if (mContext->mWindow) return;
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, val);
	mWindowSettings.stencilBits = val;
}

int Window::getStencilBits() const {
	int res = 0;
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &res);
	return res;
}

void Window::setMajorVersion(int val) {
	if (mContext->mWindow) return;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, val);
	mWindowSettings.majorVersion = val;
}

int Window::getMajorVersion() const {
	int res = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &res);
	return res;
}

void Window::setMinorVersion(int val) {
	if (mContext->mWindow) return;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, val);
	mWindowSettings.minorVersion = val;
}

int Window::getMinorVersion() const {
	int res = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &res);
	return res;
}

void Window::setAntialiasingLevel(int val) {
	if (mContext->mWindow) return;
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, val);
	mWindowSettings.antialiasingLevel = val;
}

int Window::getAntialiasingLevel() const {
	int res = 0;
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &res);
	return res;
}

void Window::setRefreshRate(int val) {
	
}

int Window::getRefreshRate() const {
	return 0;
}

void Window::setFullscreen(bool val) {
	switch (GetCurrentPlatform()) {
	case Platform::IOS:
	case Platform::ANDROIDOS: {
		//Can not change for this platform
		return;
	}
	case Platform::EMSCRIPT:
	case Platform::WINDOWS:
	case Platform::MAC:
	default: {
		break;
	}
	}
	mWindowSettings.isFullscreen = val;
	SDL_SetWindowFullscreen(mContext->mWindow, mWindowSettings.isFullscreen);
}

bool Window::getIsFullscreen() const {
	switch (GetCurrentPlatform()) {
		case Platform::IOS:
		case Platform::ANDROIDOS: {
			return true;
		}
		case Platform::EMSCRIPT:
		case Platform::WINDOWS:
		case Platform::MAC:
		default: {
			break;
		}
	}
	return mWindowSettings.isFullscreen;
}

void Window::toggleFullscreen() {
	switch (GetCurrentPlatform()) {
		case Platform::IOS:
		case Platform::ANDROIDOS: {
			//Can not change for this platform
			return;
		}
		case Platform::EMSCRIPT:
		case Platform::WINDOWS:
		case Platform::MAC:
		default: {
			break;
		}
	}
	mWindowSettings.isFullscreen = !mWindowSettings.isFullscreen;
	SDL_SetWindowFullscreen(mContext->mWindow, mWindowSettings.isFullscreen);
}

void Window::hide() const {
	SDL_HideWindow(mContext->mWindow);
}

void Window::show() const {
	SDL_ShowWindow(mContext->mWindow);
}

void Window::focus() const {
	SDL_SetWindowInputFocus(mContext->mWindow);
}

bool Window::hasFocus() const {
	return mContext->mIsFocus;
}

void Window::pollEvent() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
		ImGui_ImplSDL2_ProcessEvent(&event);
#endif
		switch (event.type) {
		case SDL_QUIT:
			mIsClose = false;
			break;
		case SDL_KEYDOWN:
			keyPressedEvent.run(event.key.keysym.scancode);
			break;
		case SDL_KEYUP:
			keyReleasedEvent.run(event.key.keysym.scancode);
			break;
		case SDL_MOUSEBUTTONUP:
			mouseButtonPressedEvent.run(event.button.button);
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouseButtonReleasedEvent.run(event.button.button);
			break;
		case SDL_MOUSEMOTION:

			break;
		//GAMEPAD
		case SDL_CONTROLLERDEVICEADDED: {
			auto gp = SDL_GameControllerOpen(event.cdevice.which);
			mContext->addGamepad(gp);
			int id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp));
			std::string name = SDL_GameControllerName(gp);
			gamepadAddEvent.run(INPUT::Gamepad(id, name));
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED: {
			for (auto gp : mContext->mGamepads) {
				if (gp && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp))) {
					SDL_GameControllerClose(gp);
					mContext->removeGamepad(gp);
					gamepadRemoveEvent.run(event.cdevice.which);
				}
			}
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN: {
			for (auto gp : mContext->mGamepads) {
				int id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp));
				if (gp && event.cdevice.which == id) {
					gamepadButtonPressedEvent.run(id, ToGamepadButton.at(static_cast<SDL_GameControllerButton>(event.cbutton.button)));
				}
			}
			break;
		}
		case SDL_CONTROLLERBUTTONUP: {
			for (auto gp : mContext->mGamepads) {
				int id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp));
				if (gp && event.cdevice.which == id) {
					gamepadButtonReleasedEvent.run(id, ToGamepadButton.at(static_cast<SDL_GameControllerButton>(event.cbutton.button)));
				}
			}
			break;
		}
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			mContext->mIsFocus = true;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			mContext->mIsFocus = false;
			break;
		default:
			break;
		}

		//GAMEPAD
		for (auto gp : mContext->mGamepads) {
			if (gp) {
				const int id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gp));

				static float lxstick = 0.0f;
				static float lystick = 0.0f;
				float x = (float)SDL_GameControllerGetAxis(gp, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX) / (float)INT16_MAX;
				float y = (float)SDL_GameControllerGetAxis(gp, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY) / (float)INT16_MAX;
				if (!MATH::CMP(lxstick, x)) gamepadAxisEvent.run(id, INPUT::Gamepad::GAMEPAD_AXIS::leftStick_X, x);
				if (!MATH::CMP(lystick, y)) gamepadAxisEvent.run(id, INPUT::Gamepad::GAMEPAD_AXIS::leftStick_Y, y);
				lxstick = x;
				lystick = y;

				static float rxstick = 0.0f;
				static float rystick = 0.0f;
				x = (float)SDL_GameControllerGetAxis(gp, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX) / (float)INT16_MAX;
				y = (float)SDL_GameControllerGetAxis(gp, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY) / (float)INT16_MAX;
				if (!MATH::CMP(rxstick, x)) gamepadAxisEvent.run(id, INPUT::Gamepad::GAMEPAD_AXIS::rightStick_X, x);
				if (!MATH::CMP(rystick, y)) gamepadAxisEvent.run(id, INPUT::Gamepad::GAMEPAD_AXIS::rightStick_Y, y);
				lxstick = x;
				lystick = y;

				static float rtrigger = 0.0f;
				static float ltrigger = 0.0f;
				x = (float)SDL_GameControllerGetAxis(gp,SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT) / (float)INT16_MAX;
				y = (float)SDL_GameControllerGetAxis(gp, SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / (float)INT16_MAX;
				if (!MATH::CMP(rtrigger, x)) gamepadTriggerEvent.run(id, INPUT::Gamepad::GAMEPAD_TRIGGER::leftTrigger, x);
				if (!MATH::CMP(ltrigger, y)) gamepadTriggerEvent.run(id, INPUT::Gamepad::GAMEPAD_TRIGGER::rightTrigger, y);
				rtrigger = x;
				ltrigger = y;
			}
		}
	}
	SDL_GL_MakeCurrent(mContext->mWindow, mContext->mContext);
}

void Window::draw() const {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}
#endif
	SDL_GL_SwapWindow(mContext->mWindow);
}

void Window::preUpdate() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
#endif
}

void Window::update() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	ImGui::Render();
#endif
}

bool Window::isClosed() const {
	return mIsClose;
}

void Window::setCursorVisible(bool isVisible, bool isLock) const {
	SDL_ShowCursor(isVisible ? SDL_ENABLE : SDL_DISABLE);
	SDL_SetRelativeMouseMode(isLock ? SDL_TRUE : SDL_FALSE);
}

std::pair<int, int> Window::getDrawableSize() {
	int viewportWidth;
	int viewportHeight;
	SDL_GL_GetDrawableSize(mContext->mWindow, &viewportWidth, &viewportHeight);
	return { viewportWidth , viewportHeight };
}

void Window::initImGUI() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)

#ifdef __EMSCRIPTEN__
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
#elif defined(__ANDROID__)
	const char* glsl_version = "#version 100";
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
#endif

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(mContext->mWindow, mContext->mContext);
	ImGui_ImplOpenGL3_Init(glsl_version);
#endif
}

WindowSettings& Window::getSetting() {
	return mWindowSettings;
}

bool shouldDisplayFullScreen() {
	switch (GetCurrentPlatform())
	{
	case Platform::IOS:
	case Platform::ANDROIDOS:
		return true;
	default:
		return false;
	}
}

void Window::create() {
	auto displaySize = getSize();

#ifdef __EMSCRIPTEN__
	// GL ES 2.0 + GLSL 100
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#elif defined(__ANDROID__)
	// GL 3.2 Core + GLSL 150
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	// GL 3.0 + GLSL 130
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_Window* _window{ SDL_CreateWindow(
		mWindowSettings.title.c_str(),
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		displaySize.x, displaySize.y,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI) };

	if (::shouldDisplayFullScreen() || mWindowSettings.isFullscreen) {
		mWindowSettings.isFullscreen = true;
		SDL_SetWindowFullscreen(_window, SDL_TRUE);
	}
	mContext->mWindow = _window;
	mContext->mContext = SDL_GL_CreateContext(mContext->mWindow);

	initImGUI();
}
#endif

#ifdef USE_GLFW

#include "renderModule/gameRendererGl.h"

#if defined(USE_EDITOR) || defined(USE_CHEATS)
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#endif

static void glfwErrorCallback(int error, const char* description) {
	LOG_ERROR << ("Glfw Error: " + std::to_string(error) + " " + description);
}

IKIGAI::WINDOW::Window::Window(const WindowSettings& mWindowSettings) : mWindowSettings(mWindowSettings) {
	create();
}
bool IKIGAI::WINDOW::Window::isClosed() const {
	return mIsClose;
}

void IKIGAI::WINDOW::Window::toggleFullscreen() {
	setFullscreen(!mWindowSettings.isFullscreen);
}

int IKIGAI::WINDOW::Window::getRefreshRate() const {
	return mWindowSettings.refreshRate;
}

bool IKIGAI::WINDOW::Window::getIsFullscreen() const {
	return mWindowSettings.isFullscreen;
}


std::string IKIGAI::WINDOW::Window::getTitle() const {
	return mWindowSettings.title;
}

IKIGAI::WINDOW::WindowSettings& IKIGAI::WINDOW::Window::getSetting() {
	return mWindowSettings;
}

IKIGAI::WINDOW::Window::~Window() {
	glfwTerminate();
}

#include "renderModule/backends/vk/frameBufferVk.h"
#include <backends/imgui_impl_vulkan.h>
#include <renderModule/gameRendererVk.h>
static ImGui_ImplVulkanH_Window g_MainWindowData;
static VkAllocationCallbacks* g_Allocator = nullptr;
static int                      g_MinImageCount = 2;

void IKIGAI::WINDOW::Window::initImGUI() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
#if defined OPENGL_BACKEND || defined  VULKAN_BACKEND
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif
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
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplGlfw_InitForVulkan(window.get(), false);
		//initForVk();
	//}
#endif
	//ArchTheme();

	// Setup Platform/Renderer backends
#ifdef OPENGL_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplGlfw_InitForOpenGL(window.get(), false);
		const char* glsl_version = "#version 330";
		ImGui_ImplOpenGL3_Init(glsl_version);
	//}
#endif

#ifdef DX12_BACKEND
	auto& gr = reinterpret_cast<RENDER::GameRendererDx12&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>());
	auto render = reinterpret_cast<RENDER::GameRendererDx12&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).mDriver;


	ImGui_ImplWin32_Init(gr.mhMainWnd);
	ImGui_ImplDX12_Init(render->mDevice.Get(), 1,
		DXGI_FORMAT_R8G8B8A8_UNORM, render->mTexturesDescHeap.Get(),
		render->mTexturesDescHeap->GetCPUDescriptorHandleForHeapStart(),
		render->mTexturesDescHeap->GetGPUDescriptorHandleForHeapStart());

#endif

#if defined  OPENGL_BACKEND || defined  VULKAN_BACKEND
	keyEvent.add([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});
	mouseButtonEvent.add([](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	});
#endif
#endif
}

void IKIGAI::WINDOW::Window::preUpdate() {
#ifdef OPENGL_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	//}
#endif
#ifdef VULKAN_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	//}
#endif

#ifdef DX12_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	//}
#endif
}

void IKIGAI::WINDOW::Window::setSize(unsigned int w, unsigned int h) {
	mWindowSettings.size = {w, h};
	glfwSetWindowSize(window.get(), w, h);
}

void IKIGAI::WINDOW::Window::setPosition(int x, int y) {
	position = {x, y};
	glfwSetWindowPos(window.get(), x, y);
}

void IKIGAI::WINDOW::Window::hide() const {
	glfwHideWindow(window.get());
}

void IKIGAI::WINDOW::Window::show() const {
	glfwShowWindow(window.get());
}

void IKIGAI::WINDOW::Window::focus() const {
	glfwFocusWindow(window.get());
}

bool IKIGAI::WINDOW::Window::hasFocus() const {
	return glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void IKIGAI::WINDOW::Window::setFullscreen(bool val) {
	mWindowSettings.isFullscreen = val;
	glfwSetWindowMonitor(
		window.get(),
		val ? glfwGetPrimaryMonitor() : nullptr,
		position.x,
		position.y,
		mWindowSettings.size.x,
		mWindowSettings.size.y,
		mWindowSettings.refreshRate
	);
}

void IKIGAI::WINDOW::Window::setCursorVisible(bool isVisible, bool isLock) const {
	auto val = GLFW_CURSOR_NORMAL;
	if (!isVisible && isLock) {
		val = GLFW_CURSOR_DISABLED;
	} else if (!isVisible) {
		val = GLFW_CURSOR_HIDDEN;
	}
	glfwSetInputMode(window.get(), GLFW_CURSOR, val);
}

void IKIGAI::WINDOW::Window::setDepthBits(int val) {
	mWindowSettings.depthBits = val;
	glfwWindowHint(GLFW_DEPTH_BITS, val);
}

int IKIGAI::WINDOW::Window::getDepathBits() const {
	return mWindowSettings.depthBits;
}

void IKIGAI::WINDOW::Window::setStencilBits(int val) {
	mWindowSettings.stencilBits = val;
	glfwWindowHint(GLFW_STENCIL_BITS, val);
}

int IKIGAI::WINDOW::Window::getStencilBits() const {
	return mWindowSettings.stencilBits;
}

void IKIGAI::WINDOW::Window::setMajorVersion(int val) {
	mWindowSettings.majorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, val);
}

int IKIGAI::WINDOW::Window::getMajorVersion() const {
	return mWindowSettings.majorVersion;
}

void IKIGAI::WINDOW::Window::setMinorVersion(int val) {
	mWindowSettings.minorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, val);
}

int IKIGAI::WINDOW::Window::getMinorVersion() const {
	return mWindowSettings.minorVersion;
}

void IKIGAI::WINDOW::Window::setAntialiasingLevel(int val) {
	mWindowSettings.antialiasingLevel = val;
	glfwWindowHint(GLFW_SAMPLES, val);
}

int IKIGAI::WINDOW::Window::getAntialiasingLevel() const {
	return mWindowSettings.antialiasingLevel;
}

void IKIGAI::WINDOW::Window::setRefreshRate(int val) {
	mWindowSettings.refreshRate = val;
	glfwWindowHint(GLFW_REFRESH_RATE, val);
}

void IKIGAI::WINDOW::Window::setTitle(const std::string& _title) {
	mWindowSettings.title = _title;
	glfwSetWindowTitle(window.get(), mWindowSettings.title.c_str());
}

void IKIGAI::WINDOW::Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		keyPressedEvent.run(key);
	} else if (action == GLFW_RELEASE) {
		keyReleasedEvent.run(key);
	}
}

void IKIGAI::WINDOW::Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		mouseButtonPressedEvent.run(button);
	} else if (action == GLFW_RELEASE) {
		mouseButtonReleasedEvent.run(button);
	}
}

void IKIGAI::WINDOW::Window::DestroyGLFW::operator()(GLFWwindow* ptr) const {
	glfwDestroyWindow(ptr);
}

void IKIGAI::WINDOW::Window::updateWindow() {
	setFullscreen(mWindowSettings.isFullscreen);
	setTitle(mWindowSettings.title);
	setMajorVersion(mWindowSettings.majorVersion);
	setMinorVersion(mWindowSettings.minorVersion);
	setAntialiasingLevel(mWindowSettings.antialiasingLevel);
	setDepthBits(mWindowSettings.depthBits);
	setStencilBits(mWindowSettings.stencilBits);
}
#include <renderModule/backends/interface/driverInterface.h>
void IKIGAI::WINDOW::Window::create() {
	glfwInit();
#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		setMajorVersion(mWindowSettings.majorVersion);
		setMinorVersion(mWindowSettings.minorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
#endif
#ifdef VULKAN_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
		if (!glfwVulkanSupported()) {
			std::cerr << "GLFW: Vulkan not supported\n" << std::endl;
			throw;
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//}
#endif
	setAntialiasingLevel(mWindowSettings.antialiasingLevel);
	setDepthBits(mWindowSettings.depthBits);
	setStencilBits(mWindowSettings.stencilBits);

	window = std::unique_ptr<GLFWwindow, DestroyGLFW>(glfwCreateWindow(
		mWindowSettings.size.x, mWindowSettings.size.y,
		mWindowSettings.title.c_str(), NULL, NULL));

#ifdef VULKAN_BACKEND
	//glfwGetFramebufferSize(window, &m_BufferWidth, &m_BufferHeight);
#endif

	glfwMakeContextCurrent(window.get());
	glfwSwapInterval(1); //vsync
	setRefreshRate(mWindowSettings.refreshRate);
	setCursorVisible(mWindowSettings.isCursorVisible, mWindowSettings.isCursorLock);

	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	} else {
		int x, y;
		glfwGetWindowPos(window.get(), &x, &y);
		position.x = x;
		position.y = y;
	}

	setFullscreen(mWindowSettings.isFullscreen);

	//TODO mode to service
	//INPUT::GamepadManager::Instance();
	//
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(window.get(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		RESOURCES::ServiceManager::Get<WINDOW::Window>().keyEvent.run(window, key, scancode, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW::Window>().keyCallback(window, key, scancode, action, mods);
	});
	glfwSetMouseButtonCallback(window.get(), [](GLFWwindow* window, int button, int action, int mods) {
		RESOURCES::ServiceManager::Get<WINDOW::Window>().mouseButtonEvent.run(window, button, action, mods);
		RESOURCES::ServiceManager::Get<WINDOW::Window>().mouseCallback(window, button, action, mods);
	});
	glfwSetWindowCloseCallback(window.get(), [](GLFWwindow* window) {
		RESOURCES::ServiceManager::Get<WINDOW::Window>().mIsClose = true;
	});

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow* window, int width, int height) {
		auto& renderer = RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>();
#ifdef OPENGL_BACKEND
	auto _renderer = reinterpret_cast<RENDER::GameRendererGl*>(&renderer);
	_renderer->resize();
#endif
#ifdef VULKAN_BACKEND
	auto _renderer = reinterpret_cast<RENDER::GameRendererVk*>(&renderer);
	_renderer->resize();
#endif
	//TODO:

		});

	initImGUI();
}

void IKIGAI::WINDOW::Window::pollEvent() {
	glfwPollEvents();
	//INPUT::GamepadManager::Instance().update([this](const INPUT::Gamepad::GamepadData& data) {
	//	gamepadEvent.run(data);
	//	});
}

void IKIGAI::WINDOW::Window::draw() const {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
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
#ifdef DX12_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {

	}
#endif
#if defined(OPENGL_BACKEND) || defined(VULKAN_BACKEND)
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
#endif
#endif

#ifdef OPENGL_BACKEND
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
		if (!glfwWindowShouldClose(window.get())) {
			glfwSwapBuffers(window.get());
		}
	}
#endif
}

void IKIGAI::WINDOW::Window::update() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	ImGui::Render();
#endif
}

GLFWwindow& IKIGAI::WINDOW::Window::getContext() const {
	return *window;
}

GLFWwindow* IKIGAI::WINDOW::Window::getContextPtr() const {
	return window.get();
}

IKIGAI::MATH::Vector2i IKIGAI::WINDOW::Window::getMousePos() const {
	double x, y;
	glfwGetCursorPos(window.get(), &x, &y);
	return IKIGAI::MATH::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
IKIGAI::MATH::Vector2u IKIGAI::WINDOW::Window::getSize() const {
	auto width = 0;
	auto height = 0;
	glfwGetWindowSize(window.get(), &width, &height);
	return IKIGAI::MATH::Vector2u(width, height);
}

IKIGAI::MATH::Vector2i IKIGAI::WINDOW::Window::getPosition() const {
	int x, y;
	glfwGetWindowPos(window.get(), &x, &y);
	return IKIGAI::MATH::Vector2i(x, y);
}
#endif

#ifdef USE_WINAPI

Window::~Window() {
	//glfwTerminate();
}


void Window::setSize(unsigned int w, unsigned int h) {
	mWindowSettings.size = {w, h};
	//glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	position = {x, y};
	//glfwSetWindowPos(window.get(), x, y);
}

void Window::hide() const {
	//glfwHideWindow(window.get());
}

void Window::show() const {
	//glfwShowWindow(window.get());
}

void Window::focus() const {
	//glfwFocusWindow(window.get());
}

bool Window::hasFocus() const {
	return true;//glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void Window::setFullscreen(bool val) {
	mWindowSettings.isFullscreen = val;
	//glfwSetWindowMonitor(
	//	window.get(),
	//	val ? glfwGetPrimaryMonitor() : nullptr,
	//	position.x,
	//	position.y,
	//	mWindowSettings.size.x,
	//	mWindowSettings.size.y,
	//	mWindowSettings.refreshRate
	//);
}

void Window::setCursorVisible(bool isVisible, bool isLock) const {
	//auto val = GLFW_CURSOR_NORMAL;
	//if (!isVisible && isLock) {
	//	val = GLFW_CURSOR_DISABLED;
	//}
	//else if (!isVisible) {
	//	val = GLFW_CURSOR_HIDDEN;
	//}
	//glfwSetInputMode(window.get(), GLFW_CURSOR, val);
}


void Window::setDepthBits(int val) {
	mWindowSettings.depthBits = val;
	glfwWindowHint(GLFW_DEPTH_BITS, val);
}

int Window::getDepathBits() const {
	return mWindowSettings.depthBits;
}

void Window::setStencilBits(int val) {
	mWindowSettings.stencilBits = val;
	glfwWindowHint(GLFW_STENCIL_BITS, val);
}

int Window::getStencilBits() const {
	return mWindowSettings.stencilBits;
}

void Window::setMajorVersion(int val) {
	mWindowSettings.majorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, val);
}

int Window::getMajorVersion() const {
	return mWindowSettings.majorVersion;
}

void Window::setMinorVersion(int val) {
	mWindowSettings.minorVersion = val;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, val);
}

int Window::getMinorVersion() const {
	return mWindowSettings.minorVersion;
}

void Window::setAntialiasingLevel(int val) {
	mWindowSettings.antialiasingLevel = val;
	glfwWindowHint(GLFW_SAMPLES, val);
}

int Window::getAntialiasingLevel() const {
	return mWindowSettings.antialiasingLevel;
}

void Window::setRefreshRate(int val) {
	mWindowSettings.refreshRate = val;
	//glfwWindowHint(GLFW_REFRESH_RATE, val);
}


void Window::setTitle(const std::string& _title) {
	mWindowSettings.title = _title;
	//glfwSetWindowTitle(window.get(), mWindowSettings.title.c_str());
}

//void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		keyPressedEvent.run(key);
//	}
//	else if (action == GLFW_RELEASE) {
//		keyReleasedEvent.run(key);
//	}
//}
//
//void Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		mouseButtonPressedEvent.run(button);
//	}
//	else if (action == GLFW_RELEASE) {
//		mouseButtonReleasedEvent.run(button);
//	}
//}

//void Window::DestroyGLFW::operator()(GLFWwindow* ptr) const {
//	//glfwDestroyWindow(ptr);
//}


void Window::updateWindow() {
	setFullscreen(mWindowSettings.isFullscreen);
	setTitle(mWindowSettings.title);
	setMajorVersion(mWindowSettings.majorVersion);
	setMinorVersion(mWindowSettings.minorVersion);
	setAntialiasingLevel(mWindowSettings.antialiasingLevel);
	setDepthBits(mWindowSettings.depthBits);
	setStencilBits(mWindowSettings.stencilBits);
}
#include <renderModule/backends/interface/driverInterface.h>
void Window::create() {

}

void Window::pollEvent() {
	//glfwPollEvents();
	INPUT::GamepadManager::Instance().update([this](const INPUT::Gamepad::GamepadData& data) {
		gamepadEvent.run(data);
		});
}

void Window::draw() const {

}

MATHGL::Vector2i Window::getMousePos() const {
	double x = 0.0, y = 0.0;
	//glfwGetCursorPos(window.get(), &x, &y);
	return MATHGL::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
MATHGL::Vector2u Window::getSize() const {
	auto width = 0;
	auto height = 0;
	//glfwGetWindowSize(window.get(), &width, &height);
	return MATHGL::Vector2u(800, 600);
}

MATHGL::Vector2i Window::getPosition() const {
	int x = 0, y = 0;
	//glfwGetWindowPos(window.get(), &x, &y);
	return MATHGL::Vector2i(x, y);
}
#endif

#ifdef OCULUS
using namespace IKIGAI;
using namespace IKIGAI::WINDOW;

static void ProcessAndroidCmd (struct android_app* app, int32_t cmd) {
    AndroidAppState* appState = (AndroidAppState*)app->userData;

    switch (cmd) {
        case APP_CMD_START:
            LOGI ("APP_CMD_START");
            break;

        case APP_CMD_RESUME:
            LOGI ("APP_CMD_RESUME");
            appState->Resumed = true;
            break;

        case APP_CMD_PAUSE:
            LOGI ("APP_CMD_PAUSE");
            appState->Resumed = false;
            break;

        case APP_CMD_STOP:
            LOGI ("APP_CMD_STOP");
            break;

        case APP_CMD_DESTROY:
            LOGI ("APP_CMD_DESTROY");
            appState->NativeWindow = NULL;
            break;

            // The window is being shown, get it ready.
        case APP_CMD_INIT_WINDOW:
            LOGI ("APP_CMD_INIT_WINDOW");
            appState->NativeWindow = app->window;
            break;

            // The window is being hidden or closed, clean it up.
        case APP_CMD_TERM_WINDOW:
            LOGI ("APP_CMD_TERM_WINDOW");
            appState->NativeWindow = NULL;
            break;
    }
}

Window::Window(const WindowSettings &p_windowSettings, android_app *app): m_app(app) {
    app->userData = &appState;
    app->onAppCmd = ProcessAndroidCmd;

    init();
}

void Window::init()
{
    void *vm    = m_app->activity->vm;
    void *clazz = m_app->activity->clazz;

    oxr_initialize_loader (vm, clazz);

    m_instance = oxr_create_instance (vm, clazz);
    m_systemId = oxr_get_system (m_instance);

    egl_init_with_pbuffer_surface (3, 24, 0, 0, 16, 16);
    oxr_confirm_gfx_requirements (m_instance, m_systemId);

    m_session    = oxr_create_session (m_instance, m_systemId);
    m_appSpace   = oxr_create_ref_space (m_session, XR_REFERENCE_SPACE_TYPE_LOCAL);
    m_stageSpace = oxr_create_ref_space (m_session, XR_REFERENCE_SPACE_TYPE_STAGE);

    m_viewSurface = oxr_create_viewsurface (m_instance, m_systemId, m_session);
}

void Window::pollEvent() {
    //TODO: send event to input system

    // Read all pending events.
    for (;;) {
        int events;
        struct android_poll_source* source;

        int timeout = -1; // blocking
        if (appState.Resumed || oxr_is_session_running() || m_app->destroyRequested)
            timeout = 0;  // non blocking

        if (ALooper_pollAll(timeout, nullptr, &events, (void**)&source) < 0) {
            break;
        }

        if (source != nullptr) {
            source->process(m_app, source);
        }
    }
}

bool Window::isClosed() const {
    return m_app->destroyRequested != 0;
}

MATH::Vector2u Window::getSize() const {
    return mSize;
}

void Window::setSize(unsigned int width, unsigned int height) {
    mSize.x = width;
    mSize.y = height;
}

void Window::preUpdate() {
    bool exit_loop, req_restart;
    oxr_poll_events (m_instance, m_session, &exit_loop, &req_restart);

    if (!oxr_is_session_running()) {
        return;
    }
}

void Window::update(std::function<void(XrCompositionLayerProjectionView &layerView,
                                       render_target_t &rtarget, XrPosef &stagePose,
                                       uint32_t viewID)> renderCb) {
    m_RenderCb = renderCb;
    std::vector<XrCompositionLayerBaseHeader*> all_layers;

    XrTime dpy_time;
    oxr_begin_frame (m_session, &dpy_time);

    std::vector<XrCompositionLayerProjectionView> projLayerViews;
    XrCompositionLayerProjection                  projLayer;
    renderLayer(dpy_time, projLayerViews, projLayer);

    all_layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&projLayer));

    /* Compose all layers */
    oxr_end_frame (m_session, dpy_time, all_layers);
}

void Window::draw() const {

}

bool Window::renderLayer(XrTime dpy_time,
                       std::vector<XrCompositionLayerProjectionView> &layerViews,
                       XrCompositionLayerProjection                  &layer)
{
    /* Acquire View Location */
    uint32_t viewCount = (uint32_t)m_viewSurface.size();

    std::vector<XrView> views(viewCount, {XR_TYPE_VIEW});
    oxr_locate_views (m_session, dpy_time, m_appSpace, &viewCount, views.data());

    layerViews.resize (viewCount);

    /* Acquire Stage Location (rerative to the View Location) */
    XrSpaceLocation stageLoc {XR_TYPE_SPACE_LOCATION};
    xrLocateSpace (m_stageSpace, m_appSpace, dpy_time, &stageLoc);


    /* Render each view */
    for (uint32_t i = 0; i < viewCount; i++) {
        XrSwapchainSubImage subImg;
        render_target_t     rtarget;

        oxr_acquire_viewsurface (m_viewSurface[i], rtarget, subImg);

        layerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        layerViews[i].pose     = views[i].pose;
        layerViews[i].fov      = views[i].fov;
        layerViews[i].subImage = subImg;

        m_RenderCb(layerViews[i], rtarget, stageLoc.pose, i);

        oxr_release_viewsurface (m_viewSurface[i]);
    }
    layer = {XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    layer.space     = m_appSpace;
    layer.viewCount = (uint32_t)layerViews.size();
    layer.views     = layerViews.data();

    return true;
}

#endif


#ifdef DX12_BACKEND

#include "imgui_impl_dx12.h"

#include "imgui_impl_win32.h"
#include "renderModule/gameRendererDx12.h"

using namespace IKIGAI;
using namespace IKIGAI::WINDOW;

Window::Window(const WindowSettings& windowSettings) : windowSettings(windowSettings) {
	create();
}
bool Window::isClosed() const {
	return isClose;
}

void Window::toggleFullscreen() {
	setFullscreen(!windowSettings.isFullscreen);
}

int Window::getRefreshRate() const {
	return windowSettings.refreshRate;
}

bool Window::getIsFullscreen() const {
	return windowSettings.isFullscreen;
}


std::string Window::getTitle() const {
	return windowSettings.title;
}

WindowSettings& Window::getSetting() {
	return windowSettings;
}


Window::~Window() {
	//glfwTerminate();
}


void Window::setSize(unsigned int w, unsigned int h) {
	windowSettings.size = {w, h};
	//glfwSetWindowSize(window.get(), w, h);
}

void Window::setPosition(int x, int y) {
	position = {x, y};
	//glfwSetWindowPos(window.get(), x, y);
}

void Window::hide() const {
	//glfwHideWindow(window.get());
}

void Window::show() const {
	//glfwShowWindow(window.get());
}

void Window::focus() const {
	//glfwFocusWindow(window.get());
}

bool Window::hasFocus() const {
	return true;//glfwGetWindowAttrib(window.get(), GLFW_FOCUSED);
}

void Window::setFullscreen(bool val) {
	windowSettings.isFullscreen = val;
	//glfwSetWindowMonitor(
	//	window.get(),
	//	val ? glfwGetPrimaryMonitor() : nullptr,
	//	position.x,
	//	position.y,
	//	windowSettings.size.x,
	//	windowSettings.size.y,
	//	windowSettings.refreshRate
	//);
}

void Window::setCursorVisible(bool isVisible, bool isLock) const {
	//auto val = GLFW_CURSOR_NORMAL;
	//if (!isVisible && isLock) {
	//	val = GLFW_CURSOR_DISABLED;
	//}
	//else if (!isVisible) {
	//	val = GLFW_CURSOR_HIDDEN;
	//}
	//glfwSetInputMode(window.get(), GLFW_CURSOR, val);
}


void Window::setDepthBits(int val) {
	windowSettings.depthBits = val;
}

int Window::getDepathBits() const {
	return windowSettings.depthBits;
}

void Window::setStencilBits(int val) {
	windowSettings.stencilBits = val;
}

int Window::getStencilBits() const {
	return windowSettings.stencilBits;
}

void Window::setMajorVersion(int val) {
	windowSettings.majorVersion = val;
}

int Window::getMajorVersion() const {
	return windowSettings.majorVersion;
}

void Window::setMinorVersion(int val) {
	windowSettings.minorVersion = val;
}

int Window::getMinorVersion() const {
	return windowSettings.minorVersion;
}

void Window::setAntialiasingLevel(int val) {
	windowSettings.antialiasingLevel = val;
}

int Window::getAntialiasingLevel() const {
	return windowSettings.antialiasingLevel;
}

void Window::setRefreshRate(int val) {
	windowSettings.refreshRate = val;
	//glfwWindowHint(GLFW_REFRESH_RATE, val);
}


void Window::setTitle(const std::string& _title) {
	windowSettings.title = _title;
	//glfwSetWindowTitle(window.get(), windowSettings.title.c_str());
}

//void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		keyPressedEvent.run(key);
//	}
//	else if (action == GLFW_RELEASE) {
//		keyReleasedEvent.run(key);
//	}
//}
//
//void Window::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		mouseButtonPressedEvent.run(button);
//	}
//	else if (action == GLFW_RELEASE) {
//		mouseButtonReleasedEvent.run(button);
//	}
//}

//void Window::DestroyGLFW::operator()(GLFWwindow* ptr) const {
//	//glfwDestroyWindow(ptr);
//}


void Window::updateWindow() {
	setFullscreen(windowSettings.isFullscreen);
	setTitle(windowSettings.title);
	setMajorVersion(windowSettings.majorVersion);
	setMinorVersion(windowSettings.minorVersion);
	setAntialiasingLevel(windowSettings.antialiasingLevel);
	setDepthBits(windowSettings.depthBits);
	setStencilBits(windowSettings.stencilBits);
}
#include <renderModule/backends/interface/driverInterface.h>
void Window::create() {
	initImGUI();
}

void Window::pollEvent() {
	//glfwPollEvents();
	//INPUT::GamepadManager::Instance().update([this](const INPUT::Gamepad::GamepadData& data) {
	//	gamepadEvent.run(data);
	//	});
}

void Window::draw() const {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {

	}
#endif
}

MATH::Vector2i Window::getMousePos() const {
	double x = 0.0, y = 0.0;
	//glfwGetCursorPos(window.get(), &x, &y);
	return MATH::Vector2i(static_cast<unsigned>(x), static_cast<unsigned>(y));
}
MATH::Vector2u Window::getSize() const {
	auto width = 0;
	auto height = 0;
	//glfwGetWindowSize(window.get(), &width, &height);
	return MATH::Vector2u(800, 600);
}

MATH::Vector2i Window::getPosition() const {
	int x = 0, y = 0;
	//glfwGetWindowPos(window.get(), &x, &y);
	return MATH::Vector2i(x, y);
}



void IKIGAI::WINDOW::Window::initImGUI() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
#if defined OPENGL_BACKEND || defined  VULKAN_BACKEND
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif
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
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::VULKAN) {
	ImGui_ImplGlfw_InitForVulkan(window.get(), false);
	//initForVk();
//}
#endif
	//ArchTheme();

	// Setup Platform/Renderer backends
#ifdef OPENGL_BACKEND
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::OPENGL) {
	ImGui_ImplGlfw_InitForOpenGL(window.get(), false);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);
	//}
#endif


#if defined  OPENGL_BACKEND || defined  VULKAN_BACKEND
	keyEvent.add([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		});
	mouseButtonEvent.add([](GLFWwindow* window, int button, int action, int mods) {
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		});
#endif
#endif
}

void IKIGAI::WINDOW::Window::preUpdate() {
	static bool isInit = false;
	if (!isInit)
	{
		isInit = true;

#ifdef DX12_BACKEND
		auto& gr = reinterpret_cast<RENDER::GameRendererDx12&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>());
		auto render = reinterpret_cast<RENDER::GameRendererDx12&>(RESOURCES::ServiceManager::Get<RENDER::GameRendererInterface>()).mDriver;


		ImGui_ImplWin32_Init(gr.mhMainWnd);
		ImGui_ImplDX12_Init(render->mDevice.Get(), 1,
			DXGI_FORMAT_R8G8B8A8_UNORM, render->mTexturesDescHeap.Get(),
			render->mTexturesDescHeap->GetCPUDescriptorHandleForHeapStart(),
			render->mTexturesDescHeap->GetGPUDescriptorHandleForHeapStart());

#endif
	}
	//if (RENDER::DriverInterface::settings.backend == RENDER::RenderSettings::Backend::DIRECTX12) {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//}
}

void Window::update() {
#if defined(USE_EDITOR) || defined(USE_CHEATS)
	ImGui::Render();
#endif
}

#endif
