#pragma once

#include <platform/system.h>

#if defined(PLATFORM_EMSCRIPTEN)
#include <platform/low_level_api.h>
#include <platform/input.h>

#include <cassert>
#include <set>

namespace Platform
{
	class SystemEmscripten : public System
	{
	public:
		SystemEmscripten(const std::string& appname);
		~SystemEmscripten();

	public:
		void process() override;
		void quit() override;

		bool isFinished() const override;

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

		void haptic(HapticType hapticType) override { /*nothing*/ }
		void alert(const std::string& text) override;
		
	private:
		std::string mAppName;
		float mScale = 1.0f;
		int mWidth = 800;
		int mHeight = 600;
		int mPrevMouseX = 0;
		int mPrevMouseY = 0;
		
	public:
		inline static void* Window = nullptr;
	};
}
#endif