#include "system_glfw.h"

#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC)

#if defined(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(PLATFORM_MAC)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <common/event_system.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Platform;

static SystemGlfw* gContext = nullptr;
static int gWidth = 800;
static int gHeight = 600;
static float gScale = 1.0f;

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CharCallback(GLFWwindow* window, unsigned int codepoint);
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void WindowSizeCallback(GLFWwindow* window, int width, int height);
static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

int main(int argc, char* argv[])
{
	sky_main();
	return 0;
}

std::shared_ptr<System> System::create(const std::string& appname)
{
#if defined(PLATFORM_WINDOWS)
	return std::make_shared<SystemWindows>(appname);
#elif defined(PLATFORM_MAC)
	return std::make_shared<SystemMac>(appname);
#endif
}

SystemGlfw::SystemGlfw(const std::string& appname) : mAppName(appname)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(gWidth, gHeight, appname.c_str(), NULL, NULL);

#if defined(PLATFORM_WINDOWS)
	mNativeWindow = glfwGetWin32Window((GLFWwindow*)mWindow);
#elif defined(PLATFORM_MAC)
	mNativeWindow = glfwGetCocoaWindow((GLFWwindow*)mWindow);
#endif

	float x_scale;
	float y_scale;

	glfwGetWindowContentScale((GLFWwindow*)mWindow, &x_scale, &y_scale);

	gScale = std::fmaxf(x_scale, y_scale);

	auto monitor = glfwGetPrimaryMonitor();
	auto video_mode = glfwGetVideoMode(monitor);

	auto window_pos_x = (video_mode->width / 2 ) - (gWidth / 2) / gScale;
	auto window_pos_y = (video_mode->height / 2) - (gHeight / 2) / gScale;

	glfwSetWindowPos((GLFWwindow*)mWindow, (int)window_pos_x, (int)window_pos_y);

	resize(gWidth, gHeight);

	glfwSetMouseButtonCallback((GLFWwindow*)mWindow, MouseButtonCallback);
	glfwSetKeyCallback((GLFWwindow*)mWindow, KeyCallback);
	glfwSetCharCallback((GLFWwindow*)mWindow, CharCallback);
	glfwSetScrollCallback((GLFWwindow*)mWindow, ScrollCallback);
	glfwSetWindowSizeCallback((GLFWwindow*)mWindow, WindowSizeCallback);
	glfwSetFramebufferSizeCallback((GLFWwindow*)mWindow, FramebufferSizeCallback);

	double mouse_x;
	double mouse_y;

	glfwGetCursorPos((GLFWwindow*)mWindow, &mouse_x, &mouse_y);

	mCursorPos = { (int)(mouse_x * gScale), (int)(mouse_y * gScale) };

	gContext = this;
}

SystemGlfw::~SystemGlfw()
{
	glfwTerminate();
}

void SystemGlfw::process()
{
	glfwPollEvents();

	double mouse_x;
	double mouse_y;

	glfwGetCursorPos((GLFWwindow*)mWindow, &mouse_x, &mouse_y);

#if defined(PLATFORM_MAC)
	auto mouse_x_i = (int)(mouse_x * gScale);
	auto mouse_y_i = (int)(mouse_y * gScale);
#elif defined(PLATFORM_WINDOWS)
	auto mouse_x_i = (int)mouse_x;
	auto mouse_y_i = (int)mouse_y;
#endif

	if (mouse_x_i != mCursorPos.x || mouse_y_i != mCursorPos.y)
	{
		mCursorPos = { mouse_x_i, mouse_y_i };

		EVENT->emit(Input::Mouse::MoveEvent{
			.pos = mCursorPos
		});
	}
}

void SystemGlfw::quit()
{
	glfwSetWindowShouldClose((GLFWwindow*)mWindow, true);
}

bool SystemGlfw::isFinished() const
{
	return glfwWindowShouldClose((GLFWwindow*)mWindow);
}

int SystemGlfw::getWidth() const
{
	return gWidth;
}

int SystemGlfw::getHeight() const
{
	return gHeight;
}

float SystemGlfw::getScale() const
{
	return gScale;
}

void SystemGlfw::setScale(float value)
{
	gScale = value;
}

bool SystemGlfw::isKeyPressed(Input::Keyboard::Key key) const
{
	static const std::unordered_map<Input::Keyboard::Key, int> KeyMap = {
		{ Input::Keyboard::Key::Backspace, GLFW_KEY_BACKSPACE },
		{ Input::Keyboard::Key::Tab, GLFW_KEY_TAB },
		{ Input::Keyboard::Key::Enter, GLFW_KEY_ENTER },
		{ Input::Keyboard::Key::LeftShift, GLFW_KEY_LEFT_SHIFT },
		{ Input::Keyboard::Key::RightShift, GLFW_KEY_RIGHT_SHIFT },
		{ Input::Keyboard::Key::LeftCtrl, GLFW_KEY_LEFT_CONTROL },
		{ Input::Keyboard::Key::RightCtrl, GLFW_KEY_RIGHT_CONTROL },
		{ Input::Keyboard::Key::LeftAlt, GLFW_KEY_LEFT_ALT },
		{ Input::Keyboard::Key::RightAlt, GLFW_KEY_RIGHT_ALT },
		{ Input::Keyboard::Key::Pause, GLFW_KEY_PAUSE },
		{ Input::Keyboard::Key::CapsLock, GLFW_KEY_CAPS_LOCK },
		{ Input::Keyboard::Key::Escape, GLFW_KEY_ESCAPE },
		{ Input::Keyboard::Key::Space, GLFW_KEY_SPACE },
		{ Input::Keyboard::Key::PageUp, GLFW_KEY_PAGE_UP },
		{ Input::Keyboard::Key::PageDown, GLFW_KEY_PAGE_DOWN },
		{ Input::Keyboard::Key::End, GLFW_KEY_END },
		{ Input::Keyboard::Key::Home, GLFW_KEY_HOME },
		{ Input::Keyboard::Key::Left, GLFW_KEY_LEFT },
		{ Input::Keyboard::Key::Up, GLFW_KEY_UP },
		{ Input::Keyboard::Key::Right, GLFW_KEY_RIGHT },
		{ Input::Keyboard::Key::Down, GLFW_KEY_DOWN },
		{ Input::Keyboard::Key::PrintScreen, GLFW_KEY_PRINT_SCREEN },
		{ Input::Keyboard::Key::Insert, GLFW_KEY_INSERT },
		{ Input::Keyboard::Key::Delete, GLFW_KEY_DELETE },

		{ Input::Keyboard::Key::A, GLFW_KEY_A },
		{ Input::Keyboard::Key::B, GLFW_KEY_B },
		{ Input::Keyboard::Key::C, GLFW_KEY_C },
		{ Input::Keyboard::Key::D, GLFW_KEY_D },
		{ Input::Keyboard::Key::E, GLFW_KEY_E },
		{ Input::Keyboard::Key::F, GLFW_KEY_F },
		{ Input::Keyboard::Key::G, GLFW_KEY_G },
		{ Input::Keyboard::Key::H, GLFW_KEY_H },
		{ Input::Keyboard::Key::I, GLFW_KEY_I },
		{ Input::Keyboard::Key::J, GLFW_KEY_J },
		{ Input::Keyboard::Key::K, GLFW_KEY_K },
		{ Input::Keyboard::Key::L, GLFW_KEY_L },
		{ Input::Keyboard::Key::M, GLFW_KEY_M },
		{ Input::Keyboard::Key::N, GLFW_KEY_N },
		{ Input::Keyboard::Key::O, GLFW_KEY_O },
		{ Input::Keyboard::Key::P, GLFW_KEY_P },
		{ Input::Keyboard::Key::Q, GLFW_KEY_Q },
		{ Input::Keyboard::Key::R, GLFW_KEY_R },
		{ Input::Keyboard::Key::S, GLFW_KEY_S },
		{ Input::Keyboard::Key::T, GLFW_KEY_T },
		{ Input::Keyboard::Key::U, GLFW_KEY_U },
		{ Input::Keyboard::Key::V, GLFW_KEY_V },
		{ Input::Keyboard::Key::W, GLFW_KEY_W },
		{ Input::Keyboard::Key::X, GLFW_KEY_X },
		{ Input::Keyboard::Key::Y, GLFW_KEY_Y },
		{ Input::Keyboard::Key::Z, GLFW_KEY_Z },

		{ Input::Keyboard::Key::NumPad0, GLFW_KEY_KP_0 },
		{ Input::Keyboard::Key::NumPad1, GLFW_KEY_KP_1 },
		{ Input::Keyboard::Key::NumPad2, GLFW_KEY_KP_2 },
		{ Input::Keyboard::Key::NumPad3, GLFW_KEY_KP_3 },
		{ Input::Keyboard::Key::NumPad4, GLFW_KEY_KP_4 },
		{ Input::Keyboard::Key::NumPad5, GLFW_KEY_KP_5 },
		{ Input::Keyboard::Key::NumPad6, GLFW_KEY_KP_6 },
		{ Input::Keyboard::Key::NumPad7, GLFW_KEY_KP_7 },
		{ Input::Keyboard::Key::NumPad8, GLFW_KEY_KP_8 },
		{ Input::Keyboard::Key::NumPad9, GLFW_KEY_KP_9 },

		{ Input::Keyboard::Key::Multiply, GLFW_KEY_KP_MULTIPLY },
		{ Input::Keyboard::Key::Add, GLFW_KEY_KP_ADD },
		{ Input::Keyboard::Key::Subtract, GLFW_KEY_KP_SUBTRACT },
		{ Input::Keyboard::Key::Decimal, GLFW_KEY_KP_DECIMAL },
		{ Input::Keyboard::Key::Divide, GLFW_KEY_KP_DIVIDE },

		{ Input::Keyboard::Key::F1, GLFW_KEY_F1 },
		{ Input::Keyboard::Key::F2, GLFW_KEY_F2 },
		{ Input::Keyboard::Key::F3, GLFW_KEY_F3 },
		{ Input::Keyboard::Key::F4, GLFW_KEY_F4 },
		{ Input::Keyboard::Key::F5, GLFW_KEY_F5 },
		{ Input::Keyboard::Key::F6, GLFW_KEY_F6 },
		{ Input::Keyboard::Key::F7, GLFW_KEY_F7 },
		{ Input::Keyboard::Key::F8, GLFW_KEY_F8 },
		{ Input::Keyboard::Key::F9, GLFW_KEY_F9 },
		{ Input::Keyboard::Key::F10, GLFW_KEY_F10 },
		{ Input::Keyboard::Key::F11, GLFW_KEY_F11 },
		{ Input::Keyboard::Key::F12, GLFW_KEY_F12 },

		{ Input::Keyboard::Key::Tilde, GLFW_KEY_GRAVE_ACCENT },
	};

	auto button = KeyMap.at(key);
	auto state = glfwGetKey((GLFWwindow*)mWindow, button);

	return state == GLFW_PRESS;
}

bool SystemGlfw::isKeyPressed(Input::Mouse::Button key) const
{
	static const std::unordered_map<Input::Mouse::Button, int> ButtonMap = {
		{ Input::Mouse::Button::Left, GLFW_MOUSE_BUTTON_LEFT },
		{ Input::Mouse::Button::Middle, GLFW_MOUSE_BUTTON_MIDDLE },
		{ Input::Mouse::Button::Right, GLFW_MOUSE_BUTTON_RIGHT },
	};

	auto button = ButtonMap.at(key);
	auto state = glfwGetMouseButton((GLFWwindow*)mWindow, button);

	return state == GLFW_PRESS;
}

void SystemGlfw::resize(int width, int height)
{
	int pos_x;
	int pos_y;
	
	glfwGetWindowPos((GLFWwindow*)mWindow, &pos_x, &pos_y);
	
	auto w_delta = gWidth - width;
	auto h_delta = gHeight - height;
	
	auto x_offset = w_delta / 2 / gScale;
	auto y_offset = h_delta / 2 / gScale;
	
	glfwSetWindowPos((GLFWwindow*)mWindow, int(pos_x + x_offset), int(pos_y + y_offset));

	width = int(width / gScale);
	height = int(height / gScale);

	glfwSetWindowSize((GLFWwindow*)mWindow, width, height);
}

void SystemGlfw::setTitle(const std::string& text)
{
	glfwSetWindowTitle((GLFWwindow*)mWindow, text.c_str());
}

void SystemGlfw::hideCursor()
{
	glfwSetInputMode((GLFWwindow*)mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void SystemGlfw::showCursor()
{
	glfwSetInputMode((GLFWwindow*)mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void SystemGlfw::setCursorPos(int x, int y)
{
	glfwSetCursorPos((GLFWwindow*)mWindow, (double)x, (double)y);
}

std::optional<glm::ivec2> SystemGlfw::getCursorPos() const
{
	return mCursorPos;
}

std::string SystemGlfw::getAppName() const
{
	return mAppName;
}

std::string SystemGlfw::getUUID() const
{
	return std::to_string(glm::linearRand(1, 10000000)); // TODO
}

void SystemGlfw::initializeBilling(const ProductsMap& products)
{
}

void SystemGlfw::purchase(const std::string& product)
{
}

void SystemGlfw::alert(const std::string& text)
{
}

void* SystemGlfw::getWindow() const
{
	return mNativeWindow;
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	static const std::unordered_map<int, Input::Mouse::ButtonEvent::Type> TypeMap = {
		{ GLFW_PRESS, Input::Mouse::ButtonEvent::Type::Pressed },
		{ GLFW_REPEAT, Input::Mouse::ButtonEvent::Type::Pressed },
		{ GLFW_RELEASE, Input::Mouse::ButtonEvent::Type::Released },
	};

	static const std::unordered_map<int, Input::Mouse::Button> ButtonMap = {
		{ GLFW_MOUSE_BUTTON_LEFT, Input::Mouse::Button::Left },
		{ GLFW_MOUSE_BUTTON_MIDDLE, Input::Mouse::Button::Middle },
		{ GLFW_MOUSE_BUTTON_RIGHT, Input::Mouse::Button::Right },
	};

	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

#if defined(PLATFORM_MAC)
	x *= gScale;
	y *= gScale;
#endif

	EVENT->emit(Input::Mouse::ButtonEvent{
		.type = TypeMap.at(action),
		.button = ButtonMap.at(button),
		.pos = { x, y }
	});
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static const std::unordered_map<int, Input::Keyboard::Key> KeyMap = {
		{ GLFW_KEY_BACKSPACE, Input::Keyboard::Key::Backspace },
		{ GLFW_KEY_TAB, Input::Keyboard::Key::Tab },
		{ GLFW_KEY_ENTER, Input::Keyboard::Key::Enter },
		{ GLFW_KEY_LEFT_SHIFT, Input::Keyboard::Key::LeftShift },
		{ GLFW_KEY_RIGHT_SHIFT, Input::Keyboard::Key::RightShift },
		{ GLFW_KEY_LEFT_CONTROL, Input::Keyboard::Key::LeftCtrl },
		{ GLFW_KEY_RIGHT_CONTROL, Input::Keyboard::Key::RightCtrl },
		{ GLFW_KEY_LEFT_ALT, Input::Keyboard::Key::LeftAlt },
		{ GLFW_KEY_RIGHT_ALT, Input::Keyboard::Key::RightAlt },
		{ GLFW_KEY_PAUSE, Input::Keyboard::Key::Pause },
		{ GLFW_KEY_CAPS_LOCK, Input::Keyboard::Key::CapsLock },
		{ GLFW_KEY_ESCAPE, Input::Keyboard::Key::Escape },
		{ GLFW_KEY_SPACE, Input::Keyboard::Key::Space },
		{ GLFW_KEY_PAGE_UP, Input::Keyboard::Key::PageUp },
		{ GLFW_KEY_PAGE_DOWN, Input::Keyboard::Key::PageDown },
		{ GLFW_KEY_END, Input::Keyboard::Key::End },
		{ GLFW_KEY_HOME, Input::Keyboard::Key::Home },
		{ GLFW_KEY_LEFT, Input::Keyboard::Key::Left },
		{ GLFW_KEY_UP, Input::Keyboard::Key::Up },
		{ GLFW_KEY_RIGHT, Input::Keyboard::Key::Right },
		{ GLFW_KEY_DOWN, Input::Keyboard::Key::Down },
		{ GLFW_KEY_PRINT_SCREEN, Input::Keyboard::Key::PrintScreen },
		{ GLFW_KEY_INSERT, Input::Keyboard::Key::Insert },
		{ GLFW_KEY_DELETE, Input::Keyboard::Key::Delete },

		{ GLFW_KEY_A, Input::Keyboard::Key::A },
		{ GLFW_KEY_B, Input::Keyboard::Key::B },
		{ GLFW_KEY_C, Input::Keyboard::Key::C },
		{ GLFW_KEY_D, Input::Keyboard::Key::D },
		{ GLFW_KEY_E, Input::Keyboard::Key::E },
		{ GLFW_KEY_F, Input::Keyboard::Key::F },
		{ GLFW_KEY_G, Input::Keyboard::Key::G },
		{ GLFW_KEY_H, Input::Keyboard::Key::H },
		{ GLFW_KEY_I, Input::Keyboard::Key::I },
		{ GLFW_KEY_J, Input::Keyboard::Key::J },
		{ GLFW_KEY_K, Input::Keyboard::Key::K },
		{ GLFW_KEY_L, Input::Keyboard::Key::L },
		{ GLFW_KEY_M, Input::Keyboard::Key::M },
		{ GLFW_KEY_N, Input::Keyboard::Key::N },
		{ GLFW_KEY_O, Input::Keyboard::Key::O },
		{ GLFW_KEY_P, Input::Keyboard::Key::P },
		{ GLFW_KEY_Q, Input::Keyboard::Key::Q },
		{ GLFW_KEY_R, Input::Keyboard::Key::R },
		{ GLFW_KEY_S, Input::Keyboard::Key::S },
		{ GLFW_KEY_T, Input::Keyboard::Key::T },
		{ GLFW_KEY_U, Input::Keyboard::Key::U },
		{ GLFW_KEY_V, Input::Keyboard::Key::V },
		{ GLFW_KEY_W, Input::Keyboard::Key::W },
		{ GLFW_KEY_X, Input::Keyboard::Key::X },
		{ GLFW_KEY_Y, Input::Keyboard::Key::Y },
		{ GLFW_KEY_Z, Input::Keyboard::Key::Z },

		{ GLFW_KEY_KP_0, Input::Keyboard::Key::NumPad0 },
		{ GLFW_KEY_KP_1, Input::Keyboard::Key::NumPad1 },
		{ GLFW_KEY_KP_2, Input::Keyboard::Key::NumPad2 },
		{ GLFW_KEY_KP_3, Input::Keyboard::Key::NumPad3 },
		{ GLFW_KEY_KP_4, Input::Keyboard::Key::NumPad4 },
		{ GLFW_KEY_KP_5, Input::Keyboard::Key::NumPad5 },
		{ GLFW_KEY_KP_6, Input::Keyboard::Key::NumPad6 },
		{ GLFW_KEY_KP_7, Input::Keyboard::Key::NumPad7 },
		{ GLFW_KEY_KP_8, Input::Keyboard::Key::NumPad8 },
		{ GLFW_KEY_KP_9, Input::Keyboard::Key::NumPad9 },

		{ GLFW_KEY_KP_MULTIPLY, Input::Keyboard::Key::Multiply },
		{ GLFW_KEY_KP_ADD, Input::Keyboard::Key::Add },
		{ GLFW_KEY_KP_SUBTRACT, Input::Keyboard::Key::Subtract },
		{ GLFW_KEY_KP_DECIMAL, Input::Keyboard::Key::Decimal },
		{ GLFW_KEY_KP_DIVIDE, Input::Keyboard::Key::Divide },

		{ GLFW_KEY_F1, Input::Keyboard::Key::F1 },
		{ GLFW_KEY_F2, Input::Keyboard::Key::F2 },
		{ GLFW_KEY_F3, Input::Keyboard::Key::F3 },
		{ GLFW_KEY_F4, Input::Keyboard::Key::F4 },
		{ GLFW_KEY_F5, Input::Keyboard::Key::F5 },
		{ GLFW_KEY_F6, Input::Keyboard::Key::F6 },
		{ GLFW_KEY_F7, Input::Keyboard::Key::F7 },
		{ GLFW_KEY_F8, Input::Keyboard::Key::F8 },
		{ GLFW_KEY_F9, Input::Keyboard::Key::F9 },
		{ GLFW_KEY_F10, Input::Keyboard::Key::F10 },
		{ GLFW_KEY_F11, Input::Keyboard::Key::F11 },
		{ GLFW_KEY_F12, Input::Keyboard::Key::F12 },

		{ GLFW_KEY_GRAVE_ACCENT, Input::Keyboard::Key::Tilde },
	};

	static const std::unordered_map<int, Input::Keyboard::Event::Type> TypeMap = {
		{ GLFW_PRESS, Input::Keyboard::Event::Type::Pressed },
		{ GLFW_REPEAT, Input::Keyboard::Event::Type::Pressed },
		{ GLFW_RELEASE, Input::Keyboard::Event::Type::Released },
	};

	if (!TypeMap.contains(action))
		return;

	if (!KeyMap.contains(key))
		return;

	Input::Keyboard::Event e;

	e.type = TypeMap.at(action);
	e.key = KeyMap.at(key);

	EVENT->emit(e);
}

static void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	EVENT->emit(Input::Keyboard::CharEvent{ codepoint });
}

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

#if defined(PLATFORM_MAC)
	x *= gScale;
	y *= gScale;

	auto scroll_x = xoffset / gScale;
	auto scroll_y = yoffset / gScale;
#elif defined(PLATFORM_WINDOWS)
	auto scroll_x = xoffset;
	auto scroll_y = yoffset;
#endif

 	EVENT->emit(Input::Mouse::ScrollEvent{
 		.pos = { x, y },
 		.scroll = { scroll_x, scroll_y }
	});
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	gWidth = width;
	gHeight = height;
	EVENT->emit(Platform::System::ResizeEvent({ width, height }));
}

#endif
