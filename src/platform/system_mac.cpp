#include "system_mac.h"

#ifdef PLATFORM_MAC

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

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
}

SystemMac::~SystemMac()
{
	glfwTerminate();
}

void SystemMac::process()
{
	glfwPollEvents();
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

#endif
