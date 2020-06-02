#pragma once

#include <core/engine.h>
#include <platform/defines.h>
#include <platform/keyboard.h>
#include <platform/mouse.h>
#include <platform/touch.h>

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <map>

#define PLATFORM ENGINE->getSystem<Platform::System>()

extern void sky_main();

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

		struct VirtualKeyboardTextChanged
		{
			std::string text;
		};

		struct VirtualKeyboardEnterPressed { };

	public:
		virtual void process() = 0;
		virtual void quit() = 0;

		virtual bool isFinished() const = 0;

		virtual int getWidth() const = 0;
		virtual int getHeight() const = 0;

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

		virtual std::string getVirtualKeyboardText() const = 0;
		virtual void setVirtualKeyboardText(const std::string& text) = 0;

	public:        
		float getLogicalWidth() const;
		float getLogicalHeight() const;

		void rescale(float value);

	public:
        using ConsumeCallback = std::function<void()>;
        using ProductsMap = std::map<std::string /*id*/, ConsumeCallback>;

	public:
		virtual void initializeBilling(const ProductsMap& products) = 0;
		virtual void purchase(const std::string& product) = 0;

	public:
		static std::shared_ptr<System> create(const std::string& appname);
	};
}
