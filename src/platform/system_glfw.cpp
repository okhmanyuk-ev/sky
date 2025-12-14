#include "system_glfw.h"

#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC)// | defined(PLATFORM_EMSCRIPTEN)

#if defined(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(PLATFORM_MAC)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#if !defined(PLATFORM_EMSCRIPTEN)
#include <GLFW/glfw3native.h>
#endif
#include <sky/utils.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Platform;

static GLFWwindow* gWindow = nullptr;
static int gWidth = 800;
static int gHeight = 600;
static float gScale = 1.0f;
static std::vector<std::string> gArguments;

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CharCallback(GLFWwindow* window, unsigned int codepoint);
static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void WindowSizeCallback(GLFWwindow* window, int width, int height);
static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
static void JoystickCallback(int id, int event);

int main(int argc, char* argv[])
{
	gArguments = std::vector<std::string>(argv, argv + argc);
	sky_main();
	return 0;
}

std::shared_ptr<System> System::create(const std::string& appname)
{
	return std::make_shared<SystemGlfw>(appname);
}

SystemGlfw::SystemGlfw(const std::string& appname) : mAppName(appname)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	gWindow = glfwCreateWindow(gWidth, gHeight, appname.c_str(), NULL, NULL);

#if defined(PLATFORM_WINDOWS)
	mNativeWindow = glfwGetWin32Window(gWindow);
#elif defined(PLATFORM_MAC)
	mNativeWindow = glfwGetCocoaWindow(gWindow);
#elif defined(PLATFORM_EMSCRIPTEN)
	mNativeWindow = gWindow;
#endif

	float x_scale;
	float y_scale;

	glfwGetWindowContentScale(gWindow, &x_scale, &y_scale);

	gScale = std::fmaxf(x_scale, y_scale);

	auto monitor = glfwGetPrimaryMonitor();
	auto video_mode = glfwGetVideoMode(monitor);

	auto window_pos_x = (video_mode->width / 2) - (gWidth / 2);
	auto window_pos_y = (video_mode->height / 2) - (gHeight / 2);

#if defined(PLATFORM_MAC)
	window_pos_x /= gScale;
	window_pos_y /= gScale;
#endif

	glfwSetWindowPos(gWindow, (int)window_pos_x, (int)window_pos_y);

	resize(gWidth, gHeight);

	glfwSetMouseButtonCallback(gWindow, MouseButtonCallback);
	glfwSetKeyCallback(gWindow, KeyCallback);
	glfwSetCharCallback(gWindow, CharCallback);
	glfwSetScrollCallback(gWindow, ScrollCallback);
	glfwSetWindowSizeCallback(gWindow, WindowSizeCallback);
	glfwSetFramebufferSizeCallback(gWindow, FramebufferSizeCallback);
	glfwSetJoystickCallback(JoystickCallback);

	double mouse_x;
	double mouse_y;

	glfwGetCursorPos(gWindow, &mouse_x, &mouse_y);

	mCursorPos = { (int)(mouse_x * gScale), (int)(mouse_y * gScale) };
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
	glfwGetCursorPos(gWindow, &mouse_x, &mouse_y);

	auto mouse_x_i = static_cast<int>(mouse_x);
	auto mouse_y_i = static_cast<int>(mouse_y);
#if defined(PLATFORM_MAC)
	mouse_x_i *= gScale;
	mouse_y_i *= gScale;
#endif

	if (mouse_x_i != mCursorPos.x || mouse_y_i != mCursorPos.y)
	{
		mCursorPos = { mouse_x_i, mouse_y_i };

		sky::Emit(Input::Mouse::MoveEvent{
			.pos = mCursorPos
		});
	}
}

void SystemGlfw::quit()
{
	glfwSetWindowShouldClose(gWindow, true);
}

bool SystemGlfw::isFinished() const
{
	return glfwWindowShouldClose(gWindow);
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
	auto state = glfwGetKey(gWindow, button);

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
	auto state = glfwGetMouseButton(gWindow, button);

	return state == GLFW_PRESS;
}

void SystemGlfw::resize(int width, int height)
{
	int pos_x;
	int pos_y;

	glfwGetWindowPos(gWindow, &pos_x, &pos_y);
	auto w_delta = gWidth - width;
	auto h_delta = gHeight - height;

	auto x_offset = w_delta / 2;
	auto y_offset = h_delta / 2;

#if defined(PLATFORM_MAC)
	x_offset /= gScale;
	y_offset /= gScale;
	width /= gScale;
	height /= gScale;
#endif

	glfwSetWindowPos(gWindow, int(pos_x + x_offset), int(pos_y + y_offset));
	glfwSetWindowSize(gWindow, width, height);
}

void SystemGlfw::setTitle(const std::string& text)
{
	glfwSetWindowTitle(gWindow, text.c_str());
}

void SystemGlfw::setCursorMode(Input::CursorMode mode)
{
	static const std::unordered_map<Input::CursorMode, int> CursorMode = {
		{ Input::CursorMode::Normal, GLFW_CURSOR_NORMAL },
		{ Input::CursorMode::Hidden, GLFW_CURSOR_HIDDEN },
		{ Input::CursorMode::Locked, GLFW_CURSOR_DISABLED }
	};
	glfwSetInputMode(gWindow, GLFW_CURSOR, CursorMode.at(mode));
}

Input::CursorMode SystemGlfw::getCursorMode() const
{
	static const std::unordered_map<int, Input::CursorMode> CursorMode = {
		{ GLFW_CURSOR_NORMAL, Input::CursorMode::Normal },
		{ GLFW_CURSOR_HIDDEN, Input::CursorMode::Hidden },
		{ GLFW_CURSOR_DISABLED, Input::CursorMode::Locked }
	};
	int mode = glfwGetInputMode(gWindow, GLFW_CURSOR);
	return CursorMode.at(mode);
}

void SystemGlfw::setCursorPos(int x, int y)
{
	glfwSetCursorPos(gWindow, (double)x, (double)y);
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

void* SystemGlfw::getNativeWindowHandle() const
{
	return mNativeWindow;
}

std::string SystemGlfw::getClipboardText() const
{
	return std::string(glfwGetClipboardString(gWindow));
}

void SystemGlfw::setClipboardText(const std::string& text)
{
	glfwSetClipboardString(gWindow, text.c_str());
}

const std::vector<std::string>& SystemGlfw::getArguments() const
{
	return gArguments;
}

void SystemGlfw::updateGamepadMapping(const char* str)
{
	glfwUpdateGamepadMappings(str);
}

bool SystemGlfw::isJoystickPresent(int index) const
{
	return glfwJoystickPresent(index) == GLFW_TRUE;
}

const unsigned char* SystemGlfw::getJoystickButtons(int jid, int* count) const
{
	return glfwGetJoystickButtons(jid, count);
}

const float* SystemGlfw::getJoystickAxes(int jid, int* count) const
{
	return glfwGetJoystickAxes(jid, count);
}

bool SystemGlfw::getGamepadState(int jid, Input::Joystick::GamepadState* state) const
{
	GLFWgamepadstate _state;
	auto result = glfwGetGamepadState(jid, &_state);
	for (int i = 0; i < 15; i++)
	{
		state->buttons[i] = _state.buttons[i];
	}
	for (int i = 0; i < 6; i++)
	{
		state->axes[i] = _state.axes[i];
	}
	return result;
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

	sky::Emit(Input::Mouse::ButtonEvent{
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
		{ GLFW_REPEAT, Input::Keyboard::Event::Type::Repeat },
		{ GLFW_RELEASE, Input::Keyboard::Event::Type::Released },
	};

	if (!TypeMap.contains(action))
		return;

	if (!KeyMap.contains(key))
		return;

	Input::Keyboard::Event e;

	e.type = TypeMap.at(action);
	e.key = KeyMap.at(key);

	sky::Emit(e);
}

static void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	sky::Emit(Input::Keyboard::CharEvent{ codepoint });
}

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

#if defined(PLATFORM_MAC)
	x *= gScale;
	y *= gScale;
	xoffset /= gScale;
	yoffset /= gScale;
#endif

	sky::Emit(Input::Mouse::ScrollEvent{
		.pos = { x, y },
		.scroll = { xoffset, yoffset }
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
	sky::Emit(Platform::System::ResizeEvent({ width, height }));
}

static void JoystickCallback(int id, int event)
{
	static const std::unordered_map<int, Input::Joystick::ChangedEvent::Type> TypeMap = {
		{ GLFW_CONNECTED, Input::Joystick::ChangedEvent::Type::Connected },
		{ GLFW_DISCONNECTED, Input::Joystick::ChangedEvent::Type::Disconnected },
	};

	if (!TypeMap.contains(event))
		return;

	sky::Emit(Input::Joystick::ChangedEvent{
		.id = id,
		.type = TypeMap.at(event)
	});
}

#endif
