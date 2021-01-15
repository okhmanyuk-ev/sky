#include "system_android.h"
#ifdef PLATFORM_ANDROID

using namespace Platform;

static jobject gSkyActivity = nullptr;
static System::ProductsMap gProductsMap;
static std::string gVirtualKeyboardText;
static bool gInitialized = false;

extern "C"
{
    void Java_com_dreamskies_sky_SkyActivity_createSkyActivity(JNIEnv* env, jobject thiz)
    {
        gSkyActivity = env->NewGlobalRef(thiz);
    }

    void Java_com_dreamskies_sky_SkyActivity_destroySkyActivity(JNIEnv* env, jobject thiz)
    {
        env->DeleteGlobalRef(gSkyActivity);
    }

    void Java_com_dreamskies_sky_SkyActivity_onConsume(JNIEnv* env, jobject thiz, jstring _id)
	{
		auto id = env->GetStringUTFChars(_id, 0);

    	SystemAndroid::ExecuteInMainLoop([id] {
			if (gProductsMap.count(id) == 0)
				return;

			gProductsMap.at(id)();
    	});
	}

	void Java_com_dreamskies_sky_SkyActivity_onKeyboardTextChanged(JNIEnv* env, jobject thiz, jstring _text)
    {
		auto text = env->GetStringUTFChars(_text, 0);

    	SystemAndroid::ExecuteInMainLoop([text] {
			gVirtualKeyboardText = text;

    		if (gInitialized)
				EVENT->emit(Platform::System::VirtualKeyboardTextChanged({ gVirtualKeyboardText }));
    	});
    }

	void Java_com_dreamskies_sky_SkyActivity_onKeyboardEnterPressed(JNIEnv* env, jobject thiz)
	{
		SystemAndroid::ExecuteInMainLoop([] {
			EVENT->emit(Platform::System::VirtualKeyboardEnterPressed());
		});
	}
}

std::shared_ptr<System> System::create(const std::string& appname)
{
	return std::make_shared<SystemAndroid>(appname);
}

void android_main(android_app* app)
{
	SystemAndroid::Instance = app;

	app->userData = nullptr;
	app->onAppCmd = SystemAndroid::handle_cmd;
	app->onInputEvent = SystemAndroid::handle_input;
	app->activity->callbacks->onContentRectChanged = SystemAndroid::handle_content_rect_changed;

	ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_KEEP_SCREEN_ON |
		AWINDOW_FLAG_FULLSCREEN, 0);

	while (!gInitialized)
	{
		if (app->destroyRequested != 0)
			return;

		int num;
		android_poll_source* source;

		ALooper_pollAll(0, nullptr, &num, (void**)&source);

		if (source != nullptr)
			source->process(app, source);
	}

	sky_main();
}

void SystemAndroid::setupScale()
{
	auto env = BeginEnv();

    auto clazz = env->GetObjectClass(gSkyActivity);

    auto getWindowManagerMethod = env->GetMethodID(clazz, "getWindowManager" , "()Landroid/view/WindowManager;");
    auto windowManager = env->CallObjectMethod(Instance->activity->clazz, getWindowManagerMethod);
    auto windowManagerClass = env->FindClass("android/view/WindowManager");
    auto getDefaultDisplayMethod = env->GetMethodID(windowManagerClass, "getDefaultDisplay" , "()Landroid/view/Display;");
    auto display = env->CallObjectMethod(windowManager, getDefaultDisplayMethod);
    auto displayClass = env->FindClass("android/view/Display");
    auto displayMetricsClass = env->FindClass("android/util/DisplayMetrics");
    auto displayMetricsConstructor = env->GetMethodID(displayMetricsClass, "<init>", "()V");
    auto displayMetrics = env->NewObject(displayMetricsClass, displayMetricsConstructor);
    auto getMetricsMethod = env->GetMethodID(displayClass, "getMetrics", "(Landroid/util/DisplayMetrics;)V");
    env->CallVoidMethod(display, getMetricsMethod, displayMetrics);
    auto densityField = env->GetFieldID(displayMetricsClass, "density", "F");
	Scale = env->GetFloatField(displayMetrics, densityField);

	EndEnv();
}

Input::Keyboard::Key translateKey(int32_t key)
{
	using Input::Keyboard::Key;
	switch (key)
	{
		case AKEYCODE_UNKNOWN:
		case AKEYCODE_SOFT_LEFT:
		case AKEYCODE_SOFT_RIGHT:
		case AKEYCODE_HOME:               return Key::None;

		case AKEYCODE_BACK:               return Key::Escape;

		case AKEYCODE_CALL:
		case AKEYCODE_ENDCALL:            return Key::None;

		case AKEYCODE_0:                  return Key::NumPad0;
		case AKEYCODE_1:                  return Key::NumPad1;
		case AKEYCODE_2:                  return Key::NumPad2;
		case AKEYCODE_3:                  return Key::NumPad3;
		case AKEYCODE_4:                  return Key::NumPad4;
		case AKEYCODE_5:                  return Key::NumPad5;
		case AKEYCODE_6:                  return Key::NumPad6;
		case AKEYCODE_7:                  return Key::NumPad7;
		case AKEYCODE_8:                  return Key::NumPad8;
		case AKEYCODE_9:                  return Key::NumPad9;

		case AKEYCODE_STAR:
		case AKEYCODE_POUND:
		case AKEYCODE_DPAD_UP:
		case AKEYCODE_DPAD_DOWN:
		case AKEYCODE_DPAD_LEFT:
		case AKEYCODE_DPAD_RIGHT:
		case AKEYCODE_DPAD_CENTER:
		case AKEYCODE_VOLUME_UP:
		case AKEYCODE_VOLUME_DOWN:
		case AKEYCODE_POWER:
		case AKEYCODE_CAMERA:
		case AKEYCODE_CLEAR:              return Key::None;

		case AKEYCODE_A:                  return Key::A;
		case AKEYCODE_B:                  return Key::B;
		case AKEYCODE_C:                  return Key::C;
		case AKEYCODE_D:                  return Key::D;
		case AKEYCODE_E:                  return Key::E;
		case AKEYCODE_F:                  return Key::F;
		case AKEYCODE_G:                  return Key::G;
		case AKEYCODE_H:                  return Key::H;
		case AKEYCODE_I:                  return Key::I;
		case AKEYCODE_J:                  return Key::J;
		case AKEYCODE_K:                  return Key::K;
		case AKEYCODE_L:                  return Key::L;
		case AKEYCODE_M:                  return Key::M;
		case AKEYCODE_N:                  return Key::N;
		case AKEYCODE_O:                  return Key::O;
		case AKEYCODE_P:                  return Key::P;
		case AKEYCODE_Q:                  return Key::Q;
		case AKEYCODE_R:                  return Key::R;
		case AKEYCODE_S:                  return Key::S;
		case AKEYCODE_T:                  return Key::T;
		case AKEYCODE_U:                  return Key::U;
		case AKEYCODE_V:                  return Key::V;
		case AKEYCODE_W:                  return Key::W;
		case AKEYCODE_X:                  return Key::X;
		case AKEYCODE_Y:                  return Key::Y;
		case AKEYCODE_Z:                  return Key::Z;
		case AKEYCODE_COMMA:              return Key::None;//Comma;
		case AKEYCODE_PERIOD:             return Key::None;//Period;
		case AKEYCODE_ALT_LEFT:           return Key::Alt;//LeftAlt;
		case AKEYCODE_ALT_RIGHT:          return Key::Alt;//RightAlt;
		case AKEYCODE_SHIFT_LEFT:         return Key::Shift;//LeftShift;
		case AKEYCODE_SHIFT_RIGHT:        return Key::Shift;//RightShift;
		case AKEYCODE_TAB:                return Key::Tab;
		case AKEYCODE_SPACE:              return Key::Space;
		case AKEYCODE_SYM:
		case AKEYCODE_EXPLORER:
		case AKEYCODE_ENVELOPE:           return Key::None;
		case AKEYCODE_ENTER:              return Key::Enter;
		case AKEYCODE_DEL:                return Key::Backspace; // delete ?
		case AKEYCODE_GRAVE:              return Key::Tilde;
		case AKEYCODE_MINUS:              return Key::Subtract;
		case AKEYCODE_EQUALS:             return Key::None;//Equal;
		case AKEYCODE_LEFT_BRACKET:       return Key::None;//LBracket;
		case AKEYCODE_RIGHT_BRACKET:      return Key::None;//RBracket;
		case AKEYCODE_BACKSLASH:          return Key::None;//Backslash;
		case AKEYCODE_SEMICOLON:          return Key::None;//Semicolon;
		case AKEYCODE_APOSTROPHE:         return Key::None;//OemQuotes;
		case AKEYCODE_SLASH:              return Key::None;//Slash;

		case AKEYCODE_AT:
		case AKEYCODE_NUM:
		case AKEYCODE_HEADSETHOOK:
		case AKEYCODE_FOCUS: // *Camera* focus
		case AKEYCODE_PLUS:
		case AKEYCODE_MENU:
		case AKEYCODE_NOTIFICATION:
		case AKEYCODE_SEARCH:
		case AKEYCODE_MEDIA_PLAY_PAUSE:
		case AKEYCODE_MEDIA_STOP:
		case AKEYCODE_MEDIA_NEXT:
		case AKEYCODE_MEDIA_PREVIOUS:
		case AKEYCODE_MEDIA_REWIND:
		case AKEYCODE_MEDIA_FAST_FORWARD:
		case AKEYCODE_MUTE:               return Key::None;

		case AKEYCODE_PAGE_UP:            return Key::PageUp;
		case AKEYCODE_PAGE_DOWN:          return Key::PageDown;

		case AKEYCODE_PICTSYMBOLS:
		case AKEYCODE_SWITCH_CHARSET:
		case AKEYCODE_BUTTON_A:
		case AKEYCODE_BUTTON_B:
		case AKEYCODE_BUTTON_C:
		case AKEYCODE_BUTTON_X:
		case AKEYCODE_BUTTON_Y:
		case AKEYCODE_BUTTON_Z:
		case AKEYCODE_BUTTON_L1:
		case AKEYCODE_BUTTON_R1:
		case AKEYCODE_BUTTON_L2:
		case AKEYCODE_BUTTON_R2:
		case AKEYCODE_BUTTON_THUMBL:
		case AKEYCODE_BUTTON_THUMBR:
		case AKEYCODE_BUTTON_START:
		case AKEYCODE_BUTTON_SELECT:
		case AKEYCODE_BUTTON_MODE:        return Key::None;
		default: return Key::None;
	}
}

int SystemAndroid::getUnicode(AInputEvent* event)
{
    auto downTime = AKeyEvent_getDownTime(event);
    auto eventTime = AKeyEvent_getEventTime(event);
    auto action = AKeyEvent_getAction(event);
    auto code = AKeyEvent_getKeyCode(event);
    auto repeat = AKeyEvent_getRepeatCount(event); // not sure!
    auto metaState = AKeyEvent_getMetaState(event);
    auto deviceId = AInputEvent_getDeviceId(event);
    auto scancode = AKeyEvent_getScanCode(event);
    auto flags = AKeyEvent_getFlags(event);
    auto source = AInputEvent_getSource(event);

	auto env = BeginEnv();

	auto keyEventClass = env->FindClass("android/view/KeyEvent");
    auto KeyEventConstructor = env->GetMethodID(keyEventClass, "<init>", "(JJIIIIIIII)V");
    auto keyEventObject = env->NewObject(keyEventClass, KeyEventConstructor, downTime, eventTime, action, code, repeat, metaState, deviceId, scancode, flags, source);

    auto getUnicodeCharMethod = env->GetMethodID(keyEventClass, "getUnicodeChar", "(I)I");
    auto result = env->CallIntMethod(keyEventObject, getUnicodeCharMethod, metaState);

    env->DeleteLocalRef(keyEventClass);
    env->DeleteLocalRef(keyEventObject);

    EndEnv();

	return result;
}

int32_t SystemAndroid::handle_key_event(android_app* app, AInputEvent* event)
{
	auto action = AKeyEvent_getAction(event);
	auto e = Input::Keyboard::Event();

	if (action == AKEY_EVENT_ACTION_DOWN)
		e.type = Input::Keyboard::Event::Type::Pressed;
	else if (action == AKEY_EVENT_ACTION_UP)
		e.type = Input::Keyboard::Event::Type::Released;

	e.key = translateKey(AKeyEvent_getKeyCode(event));
	e.asciiChar = getUnicode(event);

	//EVENT->emit(e); // new keyboard events

	if (e.key == Input::Keyboard::Key::Escape)
		return 1;

	return 0;
}

int32_t SystemAndroid::handle_motion_event(android_app* app, AInputEvent* event)
{
	auto e = Input::Touch::Event();

	auto action = AMotionEvent_getAction(event);

	if (action == AMOTION_EVENT_ACTION_DOWN)
		e.type = Input::Touch::Event::Type::Begin;
	else if (action == AMOTION_EVENT_ACTION_MOVE)
		e.type = Input::Touch::Event::Type::Continue;
	else if (action == AMOTION_EVENT_ACTION_UP)
		e.type = Input::Touch::Event::Type::End;

	e.x = AMotionEvent_getX(event, 0);
	e.y = AMotionEvent_getY(event, 0);

	EVENT->emit(e);

	return 1;
}

int32_t SystemAndroid::handle_input(android_app* app, AInputEvent* event)
{
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		return handle_key_event(app, event);
	else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
		return handle_motion_event(app, event);

	return 0;
}

void SystemAndroid::handle_cmd(android_app* app, int32_t cmd)
{
	switch (cmd)
	{
		case APP_CMD_INPUT_CHANGED:
			// Command from main thread: the AInputQueue has changed.  Upon processing
			// this command, android_app->inputQueue will be updated to the new queue
			// (or NULL).
			break;

		case APP_CMD_INIT_WINDOW:
			// Command from main thread: a new ANativeWindow is ready for use.  Upon
			// receiving this command, android_app->window will contain the new window
			// surface.

			Window = app->window;

		//	if (Window == app->window)
		//		break;


			Width = ANativeWindow_getWidth(Window);
			Height = ANativeWindow_getHeight(Window);

            setupScale();

			gInitialized = true;

			if (app->userData != nullptr)
				EVENT->emit(InitWindowEvent());

			break;

		case APP_CMD_TERM_WINDOW:
			// Command from main thread: the existing ANativeWindow needs to be
			// terminated.  Upon receiving this command, android_app->window still
			// contains the existing window; after calling android_app_exec_cmd
			// it will be set to NULL.

			if (app->userData != nullptr)
				EVENT->emit(TerminateWindowEvent());

			break;

		case APP_CMD_WINDOW_RESIZED:
			// Command from main thread: the current ANativeWindow has been resized.
			// Please redraw with its new size.
			break;

		case APP_CMD_WINDOW_REDRAW_NEEDED:
			// Command from main thread: the system needs that the current ANativeWindow
			// be redrawn.  You should redraw the window before handing this to
			// android_app_exec_cmd() in order to avoid transient drawing glitches.
			break;

		case APP_CMD_CONTENT_RECT_CHANGED:
			// Command from main thread: the content area of the window has changed,
			// such as from the soft input window being shown or hidden.  You can
			// find the new content rect in android_app::contentRect.
			break;

		case APP_CMD_GAINED_FOCUS:
		{
			// Command from main thread: the app's activity window has gained
			// input focus.

			//WindowHandle defaultWindow = { 0 };
			//m_eventQueue.postSuspendEvent(defaultWindow, Suspend::WillResume);

			break;
		}

		case APP_CMD_LOST_FOCUS:
		{
			// Command from main thread: the app's activity window has lost
			// input focus.

			//WindowHandle defaultWindow = { 0 };
			//m_eventQueue.postSuspendEvent(defaultWindow, Suspend::WillSuspend);

			break;
		}

		case APP_CMD_CONFIG_CHANGED:
			// Command from main thread: the current device configuration has changed.
			break;

		case APP_CMD_LOW_MEMORY:
			// Command from main thread: the system is running low on memory.
			// Try to reduce your memory use.
			break;

		case APP_CMD_START:
			// Command from main thread: the app's activity has been started.
			break;

		case APP_CMD_RESUME:
		{
			// Command from main thread: the app's activity has been resumed.

			//WindowHandle defaultWindow = { 0 };
			//m_eventQueue.postSuspendEvent(defaultWindow, Suspend::DidResume);

			Window = app->window;

			if (app->userData != nullptr)
				EVENT->emit(ResumeEvent());

			break;
		}

		case APP_CMD_SAVE_STATE:
			// Command from main thread: the app should generate a new saved state
			// for itself, to restore from later if needed.  If you have saved state,
			// allocate it with malloc and place it in android_app.savedState with
			// the size in android_app.savedStateSize.  The will be freed for you
			// later.
			break;

		case APP_CMD_PAUSE:
		{
			// Command from main thread: the app's activity has been paused.

			//WindowHandle defaultWindow = { 0 };
			//m_eventQueue.postSuspendEvent(defaultWindow, Suspend::DidSuspend);

			if (app->userData != nullptr)
				EVENT->emit(PauseEvent());

			break;
		}

		case APP_CMD_STOP:
			// Command from main thread: the app's activity has been stopped.
			break;

		case APP_CMD_DESTROY:
			// Command from main thread: the app's activity is being destroyed,
			// and waiting for the app thread to clean up and exit before proceeding.

			//m_eventQueue.postExitEvent();

			break;
	}
}

void SystemAndroid::handle_content_rect_changed(ANativeActivity* activity, const ARect* rect)
{
	Width = rect->right - rect->left;
	Height = rect->bottom - rect->top;
}

SystemAndroid::SystemAndroid(const std::string& appname) :
	mAppName(appname)
{
	Instance->userData = this;

	lastWidth = Width;
	lastHeight = Height;
}

SystemAndroid::~SystemAndroid()
{
	//
}

void SystemAndroid::process()
{
	if (Instance->destroyRequested != 0)
	{
	//	if (mQuitCallback)
	//		mQuitCallback();
		return;
	};

	int num;
	android_poll_source* source;

	ALooper_pollAll(0, nullptr, &num, (void**)&source);

	if (source != nullptr)
		source->process(Instance, source);

	if (lastWidth != Width || lastHeight != Height)
	{
		lastWidth = Width;
		lastHeight = Height;
		EVENT->emit(ResizeEvent({ Width, Height }));
	}

	ProcessNativeCallbacks();
}

void SystemAndroid::quit()
{
	//
}

int SystemAndroid::getHeight() const
{
	return lastHeight;
}

int SystemAndroid::getWidth() const
{
	return lastWidth;
}

std::string SystemAndroid::getAppName() const
{
	return mAppName;
}

void SystemAndroid::showVirtualKeyboard()
{
    auto env = BeginEnv();
    auto clazz = env->GetObjectClass(gSkyActivity);
    auto method = env->GetMethodID(clazz, "showKeyboard", "()V");
    env->CallVoidMethod(gSkyActivity, method);
    env->DeleteLocalRef(clazz);
    EndEnv();
}

void SystemAndroid::hideVirtualKeyboard()
{
    auto env = BeginEnv();
    auto clazz = env->GetObjectClass(gSkyActivity);
    auto method = env->GetMethodID(clazz, "hideKeyboard", "()V");
    env->CallVoidMethod(gSkyActivity, method);
    env->DeleteLocalRef(clazz);
    EndEnv();
}

bool SystemAndroid::isVirtualKeyboardOpened() const
{
	return false; // TODO
}

std::string SystemAndroid::getVirtualKeyboardText() const
{
	return gVirtualKeyboardText;
}

void SystemAndroid::setVirtualKeyboardText(const std::string& text)
{
	auto env = BeginEnv();
	auto clazz = env->GetObjectClass(gSkyActivity);
	auto method = env->GetMethodID(clazz, "setKeyboardText", "(Ljava/lang/String;)V");
	auto _text = env->NewStringUTF(text.c_str());
	env->CallVoidMethod(gSkyActivity, method, _text);
	env->DeleteLocalRef(_text);
	env->DeleteLocalRef(clazz);
	EndEnv();
}

std::string SystemAndroid::getUDID() const
{
	auto env = BeginEnv();
	auto clazz = env->GetObjectClass(gSkyActivity);
	auto method = env->GetMethodID(clazz, "getUDID", "()Ljava/lang/String;");
	auto _result = (jstring)env->CallObjectMethod(gSkyActivity, method);
	auto __result = env->GetStringUTFChars(_result, 0);
	auto result = std::string(__result);
	env->ReleaseStringUTFChars(_result, __result);
	env->DeleteLocalRef(clazz);
	EndEnv();
	return result;
}

void SystemAndroid::initializeBilling(const std::map<std::string, ConsumeCallback>& products)
{
    gProductsMap = products;

    auto env = BeginEnv();
    auto list = env->FindClass("java/util/ArrayList");
    auto init = env->GetMethodID(list, "<init>", "()V");
    auto add = env->GetMethodID(list, "add", "(Ljava/lang/Object;)Z");
    auto list_obj = env->NewObject(list, init);

    for (const auto& [product, callback]: products)
    {
        auto element = env->NewStringUTF(product.c_str());
        env->CallBooleanMethod(list_obj, add, element);
        env->DeleteLocalRef(element);
    }

    auto clazz = env->GetObjectClass(gSkyActivity);
    auto method = env->GetMethodID(clazz, "initializeBilling", "(Ljava/util/List;)V");
	env->CallVoidMethod(gSkyActivity, method, list_obj);
	env->DeleteLocalRef(clazz);
	env->DeleteLocalRef(list_obj);
	EndEnv();
}

void SystemAndroid::purchase(const std::string& product)
{
    auto env = BeginEnv();
    auto clazz = env->GetObjectClass(gSkyActivity);
    auto method = env->GetMethodID(clazz, "purchase", "(Ljava/lang/String;)V");
    auto str_arg = env->NewStringUTF(product.c_str());
    env->CallVoidMethod(gSkyActivity, method, str_arg);
    env->DeleteLocalRef(str_arg);
    env->DeleteLocalRef(clazz);
    EndEnv();
}

JNIEnv* SystemAndroid::BeginEnv()
{
    auto jvm = Instance->activity->vm;
    auto env = Instance->activity->env;

    jvm->AttachCurrentThread(&env, NULL);

    return env;
}

void SystemAndroid::EndEnv()
{
	auto jvm = Instance->activity->vm;
	jvm->DetachCurrentThread();
}

void SystemAndroid::ExecuteInMainLoop(NativeCallback callback)
{
	NativeCallbacksMutex.lock();
	NativeCallbacks.push_back(callback);
	NativeCallbacksMutex.unlock();
}

void SystemAndroid::ProcessNativeCallbacks()
{
	NativeCallbacksMutex.lock();
	for (auto callback : NativeCallbacks)
	{
		callback();
	}
	NativeCallbacks.clear();
	NativeCallbacksMutex.unlock();
}

#endif