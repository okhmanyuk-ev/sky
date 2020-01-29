#pragma once

#include <Platform/system.h>

#if defined(PLATFORM_WINDOWS)
#include <Platform/low_level_api.h>

#include <Platform/mouse.h>
#include <Platform/keyboard.h>

#include <cassert>
#include <set>

#include <Common/frame_system.h>
#include <Common/event_system.h>

namespace Platform
{
	class SystemWindows : public System
	{
	public:
		SystemWindows(const std::string& appname);
		~SystemWindows();

	public:
		void process() override;
		void quit() override;

		bool isFinished() const { return mFinished; };

		int getWidth() const override { return mWidth; }
		int getHeight() const override { return mHeight; }

		float getScale() const override { return mScale; }
		void setScale(float value) override { mScale = value; }

		bool isKeyPressed(Keyboard::Key key) const override;
		bool isKeyPressed(Mouse::Button key) const override;

		void resize(int width, int height) override;
		void setTitle(const std::string& text) override;
		void hideCursor() override;
		void showCursor() override;
		void setCursorPos(int x, int y) override;

		std::string getAppName() const override;
		std::string getAppFolder() const override;

		void showVirtualKeyboard() override { /*nothing*/ };
		void hideVirtualKeyboard() override { /*nothing*/ };
		bool isVirtualKeyboardOpened() const override { return false; };

		void initializeBilling(const std::vector<std::string>& products) { /*nothing*/ };
		void purchase(const std::string& product, std::function<void()> onSuccess, std::function<void()> onFail) { if (onSuccess) onSuccess(); };

	private:
		bool mCursorHidden = false;
		std::string mAppName;
		float mScale = 1.0f;
		int mWidth = 800;
		int mHeight = 600;
		bool mFinished = false;
		std::set<Keyboard::Key> mKeyboardKeys;
		std::set<Mouse::Button> mMouseButtons;
		
	public:
		inline static HWND Window;
		inline static HMODULE Instance;

	private:
		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		void dispatchMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void dispatchKeyboardEvent(WPARAM keyCode, bool isKeyDown);
	};
}
#endif
