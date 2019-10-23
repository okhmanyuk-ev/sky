#include "system_windows.h"
#ifdef PLATFORM_WINDOWS

using namespace Platform;

std::shared_ptr<System> System::create(const std::string& appname)
{
	return std::make_shared<SystemWindows>(appname);
}

SystemWindows::SystemWindows(const std::string& appname) : mAppName(appname)
{
	WNDCLASSEX windowClass = { };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = appname.c_str();

	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, mWidth, mHeight }; 
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int realWidth = windowRect.right - windowRect.left;
	int realHeight = windowRect.bottom - windowRect.top;

	int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	Window = CreateWindow(windowClass.lpszClassName, appname.c_str(), WS_OVERLAPPEDWINDOW,
		windowLeft, windowTop, realWidth, realHeight, nullptr, nullptr, windowClass.hInstance, this);

	Instance = GetModuleHandle(nullptr);

	ShowWindow(Window, SW_SHOWDEFAULT);
}

SystemWindows::~SystemWindows()
{
	//
}

void SystemWindows::process()
{
	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void SystemWindows::quit() 
{
	mFinished = true;
}

bool SystemWindows::isKeyPressed(Keyboard::Key key) const
{
	return mKeyboardKeys.count(key) > 0;
}

bool SystemWindows::isKeyPressed(Mouse::Button key) const 
{
	return mMouseButtons.count(key) > 0;
}

void SystemWindows::resize(int width, int height)
{
	RECT cur_rect = { };
	GetWindowRect(Window, &cur_rect);

	int cur_w = cur_rect.right - cur_rect.left;
	int cur_h = cur_rect.bottom - cur_rect.top;

	int center_x = (cur_w / 2) + cur_rect.left;
	int center_y = (cur_h / 2) + cur_rect.top;

	RECT dst_rect = { 0, 0, width, height };
	AdjustWindowRect(&dst_rect, WS_OVERLAPPEDWINDOW, FALSE);

	int dst_w = dst_rect.right - dst_rect.left;
	int dst_h = dst_rect.bottom - dst_rect.top;

	int dst_left = center_x - (dst_w / 2);
	int dst_top = center_y - (dst_h / 2);

	SetWindowPos(Window, nullptr, dst_left, dst_top, dst_w, dst_h, 0);
}

void SystemWindows::setTitle(const std::string& text)
{
	SetWindowText(Window, text.c_str());
}

void SystemWindows::hideCursor()
{
	if (mCursorHidden)
		return;

	mCursorHidden = true;
	ShowCursor(false);
}

void SystemWindows::showCursor()
{
	if (!mCursorHidden)
		return;

	mCursorHidden = false;
	ShowCursor(true);
}

void SystemWindows::setCursorPos(int x, int y)
{
	POINT p = { x, y };
	ClientToScreen(Window, &p);
	SetCursorPos(p.x, p.y);
}

std::string SystemWindows::getAppName() const
{
	return mAppName;
}

std::string SystemWindows::getAppFolder() const
{
	return getenv("APPDATA") + ("\\" + mAppName) + "\\";
}

LRESULT WINAPI SystemWindows::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto& system = *(SystemWindows*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_CREATE: {
		CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateStruct->lpCreateParams);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	//case WM_SYSCOMMAND:
	//if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
	//	break;

	//	return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		system.dispatchKeyboardEvent(wParam, msg == WM_KEYDOWN);
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEWHEEL:
		system.dispatchMouseEvent(msg, wParam, lParam);
		break;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			system.mWidth = GET_X_LPARAM(lParam);
			system.mHeight = GET_Y_LPARAM(lParam);
			EVENT->emit(ResizeEvent({ system.mWidth, system.mHeight }));
		}
		break;

	case WM_DESTROY:
		system.quit();
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

void SystemWindows::dispatchMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
	Mouse::Event evt;
		
	switch (msg)
	{
	case WM_MOUSEMOVE:
		evt.type = Mouse::Event::Type::Move;
		break;

	case WM_LBUTTONDOWN:
		evt.type = Mouse::Event::Type::ButtonDown;
		evt.button = Mouse::Button::Left;
		break;

	case WM_MBUTTONDOWN:
		evt.type = Mouse::Event::Type::ButtonDown;
		evt.button = Mouse::Button::Middle;
		break;

	case WM_RBUTTONDOWN:
		evt.type = Mouse::Event::Type::ButtonDown;
		evt.button = Mouse::Button::Right;
		break;

	case WM_LBUTTONUP:
		evt.type = Mouse::Event::Type::ButtonUp;
		evt.button = Mouse::Button::Left;
		break;

	case WM_MBUTTONUP:
		evt.type = Mouse::Event::Type::ButtonUp;
		evt.button = Mouse::Button::Middle;
		break;

	case WM_RBUTTONUP:
		evt.type = Mouse::Event::Type::ButtonUp;
		evt.button = Mouse::Button::Right;
		break;

	case WM_MOUSEWHEEL:
		evt.type = Mouse::Event::Type::Wheel;
		evt.wheelY = ((float)GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
		break;

	case WM_MOUSEHWHEEL:
		evt.type = Mouse::Event::Type::Wheel;
		evt.wheelX = ((float)GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
		break;
	}

	evt.x = GET_X_LPARAM(lParam);
	evt.y = GET_Y_LPARAM(lParam);

	if (evt.type == Mouse::Event::Type::ButtonDown)
		mMouseButtons.insert(evt.button);
	else if (evt.type == Mouse::Event::Type::ButtonUp)
		mMouseButtons.erase(evt.button);

	EVENT->emit(evt);
}

void SystemWindows::dispatchKeyboardEvent(WPARAM keyCode, bool isKeyDown)
{
	Keyboard::Event evt;

	evt.type = isKeyDown ? Keyboard::Event::Type::Pressed : Keyboard::Event::Type::Released;
	evt.key = (Keyboard::Key)keyCode;
	evt.asciiChar = 0;

	BYTE keyboardState[256];

	GetKeyboardState(keyboardState);
	
	char c[2];

	if (ToAscii((UINT)keyCode, 0, keyboardState, (WORD*)c, 0) != 0)
		evt.asciiChar = c[0];

	if (evt.type == Keyboard::Event::Type::Pressed)
		mKeyboardKeys.insert(evt.key);
	else
		mKeyboardKeys.erase(evt.key);

	EVENT->emit(evt);
}
#endif