#pragma once

#include <Core/engine.h>
#include <Platform/defines.h>
#include <Platform/keyboard.h>
#include <Platform/mouse.h>
#include <Platform/touch.h>

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace Platform
{
	class System
	{
	public:
		struct ResizeEvent
		{
			int width;
			int height;
		};

	public:
		virtual void process() = 0;
		virtual void quit() = 0;

		virtual bool isFinished() const = 0;

		virtual int getWidth() const = 0;
		virtual int getHeight() const = 0;

        float getLogicalWidth() const { return getWidth() / getScale(); };
        float getLogicalHeight() const { return getHeight() / getScale(); };

		virtual float getScale() const = 0;
		virtual void setScale(float value) = 0;

		virtual bool isKeyPressed(Keyboard::Key key) const = 0;
		virtual bool isKeyPressed(Mouse::Button key) const = 0;

		virtual void resize(int width, int height) = 0;
		virtual void setTitle(const std::string& text) = 0;
		virtual void hideCursor() = 0;
		virtual void showCursor() = 0;
		virtual void setCursorPos(int x, int y) = 0;

		virtual std::string getAppName() const = 0;
		virtual std::string getAppFolder() const = 0;

		virtual void showVirtualKeyboard() = 0;
		virtual void hideVirtualKeyboard() = 0;
		virtual bool isVirtualKeyboardOpened() const = 0;

		virtual void initializeBilling(const std::vector<std::string>& products) = 0;
		virtual void purchase(const std::string& product, std::function<void()> onSuccess, std::function<void()> onFail) = 0;

	public:
		static std::shared_ptr<System> create(const std::string& appname);
	};
}