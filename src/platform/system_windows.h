#pragma once

#include <platform/system.h>

#if defined(PLATFORM_WINDOWS)
#include <platform/low_level_api.h>
#include <platform/input.h>

#include <cassert>
#include <set>

#include <common/frame_system.h>
#include <common/event_system.h>
#include <common/actions.h>

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

		bool isFinished() const override { return mFinished; };

		int getWidth() const override { return mWidth; }
		int getHeight() const override { return mHeight; }

		float getScale() const override { return mScale; }
		void setScale(float value) override { mScale = value; }
        
        float getSafeAreaTopMargin() const override { return 0.0f; }
        float getSafeAreaBottomMargin() const override { return 0.0f; }
        float getSafeAreaLeftMargin() const override { return 0.0f; }
        float getSafeAreaRightMargin() const override { return 0.0f; }
        
		bool isKeyPressed(Input::Keyboard::Key key) const override;
		bool isKeyPressed(Input::Mouse::Button key) const override;

		void resize(int width, int height) override;
		void setTitle(const std::string& text) override;
		void hideCursor() override;
		void showCursor() override;
		void setCursorPos(int x, int y) override;

		std::string getAppName() const override;

		void showVirtualKeyboard() override { /*nothing*/ };
		void hideVirtualKeyboard() override { /*nothing*/ };
		bool isVirtualKeyboardOpened() const override { return false; };

		std::string getVirtualKeyboardText() const override { return ""; };
		void setVirtualKeyboardText(const std::string& text) override { /*nothing*/ };

		std::string getUUID() const override;

		void initializeBilling(const ProductsMap& products) override;
		void purchase(const std::string& product) override;

	private:
		bool mCursorHidden = false;
		std::string mAppName;
		float mScale = 1.0f;
		int mWidth = 800;
		int mHeight = 600;
		bool mFinished = false;
		std::set<Input::Keyboard::Key> mKeyboardKeys;
		std::set<Input::Mouse::Button> mMouseButtons;
		ProductsMap mProducts;

	public:
		inline static HWND Window;
		inline static HMODULE Instance;

	private:
		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		void dispatchMouseEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void dispatchKeyboardEvent(WPARAM keyCode, bool isKeyDown);

	public:
		void makeWindow();
		void destroyWindow();
	};
}
#endif
