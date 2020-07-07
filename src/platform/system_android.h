#pragma once

#include <platform/system.h>

#if defined(PLATFORM_ANDROID)
#include <platform/low_level_api.h>

#include <platform/touch.h>
#include <platform/keyboard.h>

#include <common/frame_system.h>
#include <common/event_system.h>

#include <mutex>

namespace Platform
{
	class SystemAndroid : public System
	{
	public:
		struct PauseEvent {}; // TODO: move this to Platform::System and make more common (eg. MinimizeEvent, MaximizeEvent)
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
        
        float getSafeAreaTopMargin() const override { return 0.0f; }
        float getSafeAreaBottomMargin() const override { return 0.0f; }
        float getSafeAreaLeftMargin() const override { return 0.0f; }
        float getSafeAreaRightMargin() const override { return 0.0f; }

		bool isKeyPressed(Keyboard::Key key) const override { return false; }
		bool isKeyPressed(Mouse::Button key) const override { return false; };

		void resize(int width, int height) override { /*nothing*/ }
		void setTitle(const std::string& text) override { /*nothing*/ }
		void hideCursor() override { /*nothing*/ }
		void showCursor() override { /*nothing*/ }
		void setCursorPos(int x, int y) override { /*nothing*/ }

		std::string getAppName() const override;
		std::string getAppFolder() const override;

		void showVirtualKeyboard() override;
		void hideVirtualKeyboard() override;
		bool isVirtualKeyboardOpened() const override;

		std::string getVirtualKeyboardText() const override;
		void setVirtualKeyboardText(const std::string& text) override;

		void initializeBilling(const ProductsMap& products) override;
		void purchase(const std::string& product) override;

	public:
		static JNIEnv* BeginEnv();
		static void EndEnv();

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

		static void setupScale();
		static int getUnicode(AInputEvent* event);

		inline static int Width = 0;
		inline static int Height = 0;
		inline static float Scale = 1.0f;

	private:
		bool mQuited = false;

	private: // mainloop callbacks
		using NativeCallback = std::function<void()>;

	public:
		static void ExecuteInMainLoop(NativeCallback callback);

	private:
		static void ProcessNativeCallbacks();

	private:
		static inline std::list<NativeCallback> NativeCallbacks;
		static inline std::mutex NativeCallbacksMutex;
	};
}
#endif
