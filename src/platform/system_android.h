#pragma once

#include <Platform/system.h>

#if defined(PLATFORM_ANDROID)
#include <Platform/low_level_api.h>

#include <Platform/touch.h>
#include <Platform/keyboard.h>

#include <Common/frame_system.h>
#include <Common/event_system.h>

namespace Platform
{
	class SystemAndroid : public System
	{
	public:
		struct PauseEvent {};
		struct ResumeEvent {};
		struct InitWindowEvent {};
		struct TerminateWindowEvent {};

	public:
		SystemAndroid(const std::string& appname);
		~SystemAndroid();

	public:
		void process() override;
		void quit() override;

		bool isFinished() const override { return false; }

		int getWidth() const override;// { return mWidth; }
		int getHeight() const override;// { return mHeight; }

		float getScale() const override { return Scale; }
		void setScale(float value) override { Scale = value; }

		bool isKeyPressed(Keyboard::Key key) const override { return false; }
		bool isKeyPressed(Mouse::Button key) const override { return false; };

		void resize(int width, int height) override { /*notging*/ }
		void setTitle(const std::string& text) override { /*notging*/ }
		void hideCursor() override { /*notging*/ }
		void showCursor() override { /*notging*/ }
		void setCursorPos(int x, int y) override { /*notging*/ }

		std::string getAppName() const override;
		std::string getAppFolder() const override;

		void showVirtualKeyboard() override { setKeyboardVisible(true); };
		void hideVirtualKeyboard() override { setKeyboardVisible(false); };
		bool isVirtualKeyboardOpened() const override { return false; }; // TODO

		void initializeBilling(const ProductsMap& products) override;
		void purchase(const std::string& product) override;

	private:
		JNIEnv* getEnv() const;

	public:
		static void handle_cmd(android_app* app, int32_t cmd);
		static int32_t handle_input(android_app* app, AInputEvent* event);
		static void handle_content_rect_changed(ANativeActivity* activity, const ARect* rect);
		static int32_t handle_key_event(android_app* app, AInputEvent* event);
		static int32_t handle_motion_event(android_app* app, AInputEvent* event);

	public:
		inline static android_app* Instance = nullptr;
		inline static ANativeWindow* Window = nullptr;

	private:
		std::string mAppName;
		int lastWidth = 0;
		int lastHeight = 0;

		static void calcScale();
		static int getUnicode(AInputEvent* event);

		inline static int Width = 0;
		inline static int Height = 0;
		inline static float Scale = 1.0f;
		static void setKeyboardVisible(bool visible);

	private:
		bool mQuited = false;
	};
}
#endif