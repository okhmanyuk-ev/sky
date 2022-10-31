#include "system_mac.h"

#ifdef PLATFORM_MAC

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <common/event_system.h>

using namespace Platform;

static SystemMac* gContext = nullptr;

int main(int argc, char* argv[])
{
	sky_main();
	return 0;
}

std::shared_ptr<System> System::create(const std::string& appname)
{
	return std::make_shared<SystemMac>(appname);
}

SystemMac::SystemMac(const std::string& appname) : mAppName(appname)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mWindow = glfwCreateWindow(mWidth, mHeight, appname.c_str(), NULL, NULL);

	int count = 0;
	auto monitors = glfwGetMonitors(&count);

	auto video_mode = glfwGetVideoMode(monitors[0]);

	auto window_pos_x = (video_mode->width / 2) - (mWidth / 2);
	auto window_pos_y = (video_mode->height / 2) - (mHeight / 2);

	glfwSetWindowPos(mWindow, window_pos_x, window_pos_y);

	Window = glfwGetCocoaWindow(mWindow);
	
	float x_scale;
	float y_scale;
	
	glfwGetWindowContentScale(mWindow, &x_scale, &y_scale);
	
	mScale = std::fmaxf(x_scale, y_scale);
	
	glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
	glfwSetKeyCallback(mWindow, KeyCallback);
	glfwSetCharCallback(mWindow, CharCallback);
	glfwSetScrollCallback(mWindow, ScrollCallback);
	glfwSetWindowSizeCallback(mWindow, WindowSizeCallback);
	
	double mouse_x;
	double mouse_y;
	
	glfwGetCursorPos(mWindow, &mouse_x, &mouse_y);
	
	mPrevMouseX = (int)mouse_x;
	mPrevMouseY = (int)mouse_y;
	
	gContext = this;
}

SystemMac::~SystemMac()
{
	glfwTerminate();
}

void SystemMac::process()
{
	glfwPollEvents();

	double mouse_x;
	double mouse_y;

	glfwGetCursorPos(mWindow, &mouse_x, &mouse_y);

	auto mouse_x_i = (int)mouse_x;
	auto mouse_y_i = (int)mouse_y;

	if (mouse_x_i != mPrevMouseX || mouse_y_i != mPrevMouseY)
	{
		mPrevMouseX = mouse_x_i;
		mPrevMouseY = mouse_y_i;
		
		Input::Mouse::Event e;

		e.type = Input::Mouse::Event::Type::Move;
		e.x = mouse_x_i;
		e.y = mouse_y_i;

		EVENT->emit(e);
	}
}

void SystemMac::quit()
{
	glfwSetWindowShouldClose((GLFWwindow*)mWindow, true);
}

bool SystemMac::isFinished() const
{
	return glfwWindowShouldClose((GLFWwindow*)mWindow);
}

bool SystemMac::isKeyPressed(Input::Keyboard::Key key) const
{
	static const std::unordered_map<Input::Keyboard::Key, int> KeyMap = {
		{ Input::Keyboard::Key::Backspace, GLFW_KEY_BACKSPACE },
		{ Input::Keyboard::Key::Tab, GLFW_KEY_TAB },
		{ Input::Keyboard::Key::Enter, GLFW_KEY_ENTER },
		{ Input::Keyboard::Key::Shift, GLFW_KEY_LEFT_SHIFT },
		{ Input::Keyboard::Key::Shift, GLFW_KEY_RIGHT_SHIFT },
		{ Input::Keyboard::Key::Ctrl, GLFW_KEY_LEFT_CONTROL },
		{ Input::Keyboard::Key::Ctrl, GLFW_KEY_RIGHT_CONTROL },
		{ Input::Keyboard::Key::Alt, GLFW_KEY_LEFT_ALT },
		{ Input::Keyboard::Key::Alt, GLFW_KEY_RIGHT_ALT },
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
	auto state = glfwGetKey(mWindow, button);

	return state == GLFW_PRESS;
}

bool SystemMac::isKeyPressed(Input::Mouse::Button key) const
{
	static const std::unordered_map<Input::Mouse::Button, int> ButtonMap = {
		{ Input::Mouse::Button::Left, GLFW_MOUSE_BUTTON_LEFT },
		{ Input::Mouse::Button::Middle, GLFW_MOUSE_BUTTON_MIDDLE },
		{ Input::Mouse::Button::Right, GLFW_MOUSE_BUTTON_RIGHT },
	};

	auto button = ButtonMap.at(key);
	auto state = glfwGetMouseButton(mWindow, button);

	return state == GLFW_PRESS;
}

void SystemMac::resize(int width, int height)
{
	int pos_x;
	int pos_y;
	
	glfwGetWindowPos(mWindow, &pos_x, &pos_y);
	
	auto w_delta = mWidth - width;
	auto h_delta = mHeight - height;
	
	auto x_offset = w_delta / 2;
	auto y_offset = h_delta / 2;
	
	glfwSetWindowPos(mWindow, pos_x + x_offset, pos_y + y_offset);

	glfwSetWindowSize(mWindow, width, height);
}

void SystemMac::setTitle(const std::string& text)
{
	glfwSetWindowTitle(mWindow, text.c_str());
}

void SystemMac::hideCursor()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void SystemMac::showCursor()
{
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void SystemMac::setCursorPos(int x, int y)
{
	glfwSetCursorPos(mWindow, (double)x, (double)y);
}

std::string SystemMac::getAppName() const
{
	return mAppName;
}

std::string SystemMac::getUUID() const
{
	return ""; // TODO
}

void SystemMac::initializeBilling(const ProductsMap& products)
{
}

void SystemMac::purchase(const std::string& product)
{
}

void SystemMac::alert(const std::string& text)
{
}

void SystemMac::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	Input::Mouse::Event e;

	static const std::unordered_map<int, Input::Mouse::Event::Type> TypeMap = {
		{ GLFW_PRESS, Input::Mouse::Event::Type::ButtonDown },
		{ GLFW_RELEASE, Input::Mouse::Event::Type::ButtonUp },
	};
	
	static const std::unordered_map<int, Input::Mouse::Button> ButtonMap = {
		{ GLFW_MOUSE_BUTTON_LEFT, Input::Mouse::Button::Left },
		{ GLFW_MOUSE_BUTTON_MIDDLE, Input::Mouse::Button::Middle },
		{ GLFW_MOUSE_BUTTON_RIGHT, Input::Mouse::Button::Right },
	};

	double x;
	double y;
	
	glfwGetCursorPos(window, &x, &y);

	e.type = TypeMap.at(action);
	e.button = ButtonMap.at(button);
	e.x = (int)x;
	e.y = (int)y;

	EVENT->emit(e);
}

void SystemMac::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static const std::unordered_map<int, Input::Keyboard::Key> KeyMap = {
		{ GLFW_KEY_BACKSPACE, Input::Keyboard::Key::Backspace },
		{ GLFW_KEY_TAB, Input::Keyboard::Key::Tab },
		{ GLFW_KEY_ENTER, Input::Keyboard::Key::Enter },
		{ GLFW_KEY_LEFT_SHIFT, Input::Keyboard::Key::Shift },
		{ GLFW_KEY_RIGHT_SHIFT, Input::Keyboard::Key::Shift },
		{ GLFW_KEY_LEFT_CONTROL, Input::Keyboard::Key::Ctrl },
		{ GLFW_KEY_RIGHT_CONTROL, Input::Keyboard::Key::Ctrl },
		{ GLFW_KEY_LEFT_ALT, Input::Keyboard::Key::Alt },
		{ GLFW_KEY_RIGHT_ALT, Input::Keyboard::Key::Alt },
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

void SystemMac::CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	EVENT->emit(Input::Keyboard::CharEvent{ codepoint });
}

void SystemMac::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Input::Mouse::Event e;

	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

	e.type = Input::Mouse::Event::Type::Wheel;
	e.x = (int)x;
	e.y = (int)y;
	e.wheelX = (float)xoffset;
	e.wheelY = (float)yoffset;

 	EVENT->emit(e);
}

void SystemMac::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	gContext->mWidth = width;
	gContext->mHeight = height;
	EVENT->emit(ResizeEvent({ width, height }));
}

#endif
