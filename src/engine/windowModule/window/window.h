#pragma once
#include <string>

#include <utilsModule/event.h>
#include <mathModule/math.h>

#include "windowModule/inputManager/gamepad/gamepad.h"

//need for right including dependencies
//#include <GLFW/glfw3.h>
#ifdef OCULUS

#include "util_egl.h"
#include "util_oxr.h"
#endif

#ifdef USE_GLFW
#include <coreModule/graphicsWrapper.hpp>
#include "GLFW/glfw3.h"
#endif

namespace IKIGAI::WINDOW {
	struct WindowSettings {
		bool isFullscreen = false;
		bool isCursorVisible = true;
		bool isCursorLock = true;
		int depthBits = 24;
		int stencilBits = 8;
		int antialiasingLevel = 4;
		int majorVersion = 4;
		int minorVersion = 3;
		std::string title;
		MATH::Vector2u size = MATH::Vector2u(800, 600);
		int refreshRate = 60;

		template<class Context>
		constexpr static auto serde(Context& context, WindowSettings& value) {
			using Self = WindowSettings;
			using namespace serde::attribute;
			serde::serde_struct(context, value)
				.field(&Self::isFullscreen, "IsFullscreen")
				.field(&Self::isCursorVisible, "IsCursorVisible")
				.field(&Self::isCursorLock, "IsCursorLock")
				.field(&Self::depthBits, "DepthBits")
				.field(&Self::stencilBits, "StencilBits")
				.field(&Self::antialiasingLevel, "AntialiasingLevel")
				.field(&Self::majorVersion, "MajorVersion")
				.field(&Self::minorVersion, "MinorVersion")
				.field(&Self::title, "Title")
				.field(&Self::refreshRate, "RefreshRate")
				.field(&Self::size, "Size");
		}
	};
	
#ifdef USE_SDL
	class  Window {
	public:
		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;

		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_BUTTON> gamepadButtonPressedEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_BUTTON> gamepadButtonReleasedEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_AXIS, float> gamepadAxisEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_TRIGGER, float> gamepadTriggerEvent;

		EVENT::Event<INPUT::Gamepad> gamepadAddEvent;
		EVENT::Event<int> gamepadRemoveEvent;

		explicit Window(const WindowSettings& p_windowSettings);
		Window() = delete;
		~Window();

		[[nodiscard]] MATH::Vector2i getMousePos() const;
		void setSize(unsigned int width, unsigned int height);
		[[nodiscard]] MATH::Vector2u getSize() const;
		void setPosition(int x, int y);
		[[nodiscard]] MATH::Vector2i getPosition() const;
		void setTitle(const std::string& title);
		[[nodiscard]] std::string getTitle() const;
		void setDepthBits(int val);
		[[nodiscard]] int getDepathBits() const;
		void setStencilBits(int val);
		[[nodiscard]] int getStencilBits() const;
		void setMajorVersion(int val);
		[[nodiscard]] int getMajorVersion() const;
		void setMinorVersion(int val);
		[[nodiscard]] int getMinorVersion() const;
		void setAntialiasingLevel(int val);
		[[nodiscard]] int getAntialiasingLevel() const;
		void setRefreshRate(int val);
		[[nodiscard]] int getRefreshRate() const;
		void setFullscreen(bool val);
		[[nodiscard]] bool getIsFullscreen() const;
		void toggleFullscreen();

		void hide() const;
		void show() const;
		void focus() const;
		[[nodiscard]] bool hasFocus() const;

		void pollEvent();
		void draw() const;
		void preUpdate();
		void update();
		[[nodiscard]] bool isClosed() const;

		void setCursorVisible(bool isVisible, bool isLock) const;

		std::pair<int, int> getDrawableSize();
	private:

		void initImGUI();
		[[nodiscard]] WindowSettings& getSetting();
		
		void create();
		WindowSettings mWindowSettings;
		struct Internal;
		std::unique_ptr<Internal> mContext;
		bool mIsClose = false;
	};

#endif

#ifdef USE_GLFW
	class Window {
	public:
		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;
		EVENT::Event<INPUT::Gamepad::GamepadData> gamepadEvent;

		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_BUTTON> gamepadButtonPressedEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_BUTTON> gamepadButtonReleasedEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_AXIS, float> gamepadAxisEvent;
		EVENT::Event<int, INPUT::Gamepad::GAMEPAD_TRIGGER, float> gamepadTriggerEvent;

		explicit Window(const WindowSettings& p_windowSettings);
		Window() = delete;
		~Window();
		void initImGUI();
		void preUpdate();

		MATH::Vector2i getMousePos() const;
		void setSize(unsigned int width, unsigned int height);
		MATH::Vector2u getSize() const;
		void setPosition(int x, int y);
		MATH::Vector2i getPosition() const;
		void setTitle(const std::string& title);
		[[nodiscard]] std::string getTitle() const;
		void setDepthBits(int val);
		[[nodiscard]] int getDepathBits() const;
		void setStencilBits(int val);
		[[nodiscard]] int getStencilBits() const;
		void setMajorVersion(int val);
		[[nodiscard]] int getMajorVersion() const;
		void setMinorVersion(int val);
		[[nodiscard]] int getMinorVersion() const;
		void setAntialiasingLevel(int val);
		[[nodiscard]] int getAntialiasingLevel() const;
		void setRefreshRate(int val);
		[[nodiscard]] int getRefreshRate() const;
		void setFullscreen(bool val);
		[[nodiscard]] bool getIsFullscreen() const;
		void toggleFullscreen();

		void hide() const;
		void show() const;
		void focus() const;
		[[nodiscard]] bool hasFocus() const;
		void pollEvent();
		void draw() const;
		void update();

#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)
		EVENT::Event<GLFWwindow*, int, int, int, int> keyEvent;
		EVENT::Event<GLFWwindow*, int, int, int> mouseButtonEvent;
		[[nodiscard]] GLFWwindow& getContext() const;
		GLFWwindow* getContextPtr() const;
#endif
		[[nodiscard]] bool isClosed() const;

		void setCursorVisible(bool isVisible, bool isLock) const;

	private:


		[[nodiscard]] WindowSettings& getSetting();;
		void updateWindow();

		void create();
		WindowSettings mWindowSettings;
#if defined(VULKAN_BACKEND) || defined(OPENGL_BACKEND)
		struct DestroyGLFW {
			void operator()(GLFWwindow* ptr) const;
		};
		void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void mouseCallback(GLFWwindow* window, int button, int action, int mods);

		std::unique_ptr<GLFWwindow, DestroyGLFW> window;
#endif
		MATH::Vector2i position{};
		bool mIsClose = false;
	};
#endif


#ifdef DX12_BACKEND
	class Window {
	public:
		EVENT::Event<int> keyPressedEvent;
		EVENT::Event<int> keyReleasedEvent;
		EVENT::Event<int> mouseButtonPressedEvent;
		EVENT::Event<int> mouseButtonReleasedEvent;
		EVENT::Event<INPUT::Gamepad::GamepadData> gamepadEvent;

		explicit Window(const WindowSettings& p_windowSettings);
		Window() = delete;
		~Window();

		MATH::Vector2i getMousePos() const;
		void setSize(unsigned int width, unsigned int height);
		MATH::Vector2u getSize() const;
		void setPosition(int x, int y);
		MATH::Vector2i getPosition() const;
		void initImGUI();
		void preUpdate();
		void update();
		void setTitle(const std::string& title);
		[[nodiscard]] std::string getTitle() const;
		void setDepthBits(int val);
		[[nodiscard]] int getDepathBits() const;
		void setStencilBits(int val);
		[[nodiscard]] int getStencilBits() const;
		void setMajorVersion(int val);
		[[nodiscard]] int getMajorVersion() const;
		void setMinorVersion(int val);
		[[nodiscard]] int getMinorVersion() const;
		void setAntialiasingLevel(int val);
		[[nodiscard]] int getAntialiasingLevel() const;
		void setRefreshRate(int val);
		[[nodiscard]] int getRefreshRate() const;
		void setFullscreen(bool val);
		[[nodiscard]] bool getIsFullscreen() const;
		void toggleFullscreen();

		void hide() const;
		void show() const;
		void focus() const;
		[[nodiscard]] bool hasFocus() const;
		void pollEvent();
		void draw() const;
		[[nodiscard]] bool isClosed() const;

		void setCursorVisible(bool isVisible, bool isLock) const;
	private:


		[[nodiscard]] WindowSettings& getSetting();;
		void updateWindow();

		void create();
		WindowSettings windowSettings;
		MATH::Vector2i position{};
		bool isClose = false;
	};
#endif


#ifdef OCULUS

#ifdef OCULUS
    struct AndroidAppState {
        ANativeWindow* NativeWindow = nullptr;
        bool Resumed = false;
    };
#endif
    class Window {
		//friend DEBUG::DebugRender;
        bool renderLayer(XrTime dpy_time,
                               std::vector<XrCompositionLayerProjectionView> &layerViews,
                               XrCompositionLayerProjection                  &layer);
	public:

		explicit Window(const WindowSettings& p_windowSettings, android_app* app);
        void init();

		Window() = delete;
		~Window() = default;

		//MATHGL::Vector2i getMousePos() const;
		void setSize(unsigned int width, unsigned int height);
		MATH::Vector2u getSize() const;

		[[nodiscard]] bool getIsFullscreen() const { return true; };
		void toggleFullscreen() {};

		//void hide() const;
		//void show() const;
		//void focus() const;
		//[[nodiscard]] bool hasFocus() const;
		void pollEvent();

        void preUpdate();
        void update(std::function<void(XrCompositionLayerProjectionView &layerView,
                                       render_target_t &rtarget, XrPosef &stagePose,
                                       uint32_t viewID)> renderCb);
		void draw() const;
		//EVENT::Event<GLFWwindow*, int, int, int, int> keyEvent;
		//EVENT::Event<GLFWwindow*, int, int, int> mouseButtonEvent;
		//[[nodiscard]] GLFWwindow& getContext() const;
		[[nodiscard]] bool isClosed() const;

		//void setCursorVisible(bool isVisible, bool isLock) const;

		//std::pair<int, int> getDrawableSize();
	private:

		MATH::Vector2u mSize;

        struct android_app  *m_app = nullptr;

        XrInstance          m_instance;
        XrSession           m_session;
        XrSpace             m_appSpace;
        XrSpace             m_stageSpace;
        XrSystemId          m_systemId;
        std::vector<viewsurface_t> m_viewSurface;

        AndroidAppState appState;

        std::function<void(XrCompositionLayerProjectionView &layerView,
                           render_target_t &rtarget, XrPosef &stagePose,
                           uint32_t viewID)> m_RenderCb;
	};
#endif
}
