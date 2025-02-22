#pragma once

#include <platform/system.h>

#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC)// | defined(PLATFORM_EMSCRIPTEN)

#include <platform/low_level_api.h>
#include <platform/input.h>

#include <cassert>
#include <set>

namespace Platform
{
	class SystemGlfw : public System
	{
	public:
		SystemGlfw(const std::string& appname);
		~SystemGlfw();

	public:
		void process() override;
		void quit() override;

		bool isFinished() const override;

		int getWidth() const override;
		int getHeight() const override;

		float getScale() const override;
		void setScale(float value) override;

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

		std::optional<glm::ivec2> getCursorPos() const override;

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

		void* getNativeWindowHandle() const override;

		std::string getClipboardText() const override;
		void setClipboardText(const std::string& text) override;

		const std::vector<std::string>& getArguments() const override;

	private:
		std::string mAppName;
		glm::ivec2 mCursorPos = { 0, 0 };
		void* mNativeWindow = nullptr;
	};
}
#endif
