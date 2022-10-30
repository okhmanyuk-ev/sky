#include "system_mac.h"

#ifdef PLATFORM_MAC

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <common/event_system.h>

using namespace Platform;

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
	glfwSetScrollCallback(mWindow, ScrollCallback);
	glfwSetWindowSizeCallback(mWindow, WindowSizeCallback);
	
	double mouse_x;
	double mouse_y;
	
	glfwGetCursorPos(mWindow, &mouse_x, &mouse_y);
	
	mPrevMouseX = (int)mouse_x;
	mPrevMouseY = (int)mouse_y;
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
	return false;
}

bool SystemMac::isKeyPressed(Input::Mouse::Button key) const
{
	return false;
}

void SystemMac::resize(int width, int height)
{
//	glfwSetWindowSize(mWindow, width, height);
//	mWidth = width;
//	mHeight = height;
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
	/*Input::Keyboard::Event e;
	
	static const std::unordered_map<int, Input::Keyboard::Event::Type> TypeMap = {
		{ GLFW_PRESS, Input::Keyboard::Event::Type::Pressed },
		{ GLFW_RELEASE, Input::Keyboard::Event::Type::Released },
	};

	e.type = TypeMap.at(action);

	EVENT->emit(e);*/
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
	//mWidth = width;
	//mHeight = height;
	//EVENT->emit(ResizeEvent({ width, height }));
}

#endif
