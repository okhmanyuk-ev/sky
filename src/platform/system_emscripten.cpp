#include "system_emscripten.h"

#ifdef PLATFORM_EMSCRIPTEN

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <sky/utils.h>

using namespace Platform;

static SystemEmscripten* gContext = nullptr;
static SDL_Window* gWindow = nullptr;
static std::vector<std::string> gArguments;

static std::unordered_map<SDL_JoystickID, SDL_GameController*> gGamepads;

int main(int argc, char* argv[])
{
	gArguments = std::vector<std::string>(argv, argv + argc);
	sky_main();
	return 0;
}

std::shared_ptr<System> System::create(const std::string& appname)
{
	return std::make_shared<SystemEmscripten>(appname);
}

SystemEmscripten::SystemEmscripten(const std::string& appname) : mAppName(appname)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	gWindow = SDL_CreateWindow(appname.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWidth, mHeight, window_flags);

	gContext = this;

	float w_scale;
	float h_scale;
	SDL_GetDisplayDPI(0, NULL, &w_scale, &h_scale);

	mScale = w_scale / 96.0f;
}

SystemEmscripten::~SystemEmscripten()
{
}

void SystemEmscripten::process()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEMOTION)
		{
			mCursorPos = {
				(int)((float)event.motion.x * mScale),
				(int)((float)event.motion.y * mScale)
			};

			sky::Emit(Input::Mouse::MoveEvent{
				.pos = mCursorPos
			});
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
		{
			static const std::unordered_map<uint8_t, Input::Mouse::Button> ButtonMap = {
				{ SDL_BUTTON_LEFT, Input::Mouse::Button::Left },
				{ SDL_BUTTON_MIDDLE, Input::Mouse::Button::Middle },
				{ SDL_BUTTON_RIGHT, Input::Mouse::Button::Right },
			};

			static const std::unordered_map<uint32_t, Input::Mouse::ButtonEvent::Type> TypeMap = {
				{ SDL_MOUSEBUTTONDOWN, Input::Mouse::ButtonEvent::Type::Pressed },
				{ SDL_MOUSEBUTTONUP, Input::Mouse::ButtonEvent::Type::Released }
			};

			if (!ButtonMap.contains(event.button.button))
				continue;

			sky::Emit(Input::Mouse::ButtonEvent{
				.type = TypeMap.at(event.type),
				.button = ButtonMap.at(event.button.button),
				.pos = {
					(int)((float)event.button.x * mScale),
					(int)((float)event.button.y * mScale)
				}
			});
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
			int x = 0;
			int y = 0;

			SDL_GetMouseState(&x, &y);

			sky::Emit(Input::Mouse::ScrollEvent{
				.pos = {
					(int)((float)x * mScale),
					(int)((float)y * mScale)
				},
				.scroll = {
					event.wheel.preciseX,
					event.wheel.preciseY
				}
			});
		}
		else if (event.type == SDL_TEXTINPUT)
		{
			sky::Emit(Input::Keyboard::CharEvent{
				.codepoint = *(char32_t*)&event.text.text
			});
		}
		else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		{
			static const std::unordered_map<SDL_Keycode, Input::Keyboard::Key> KeyMap = {
				{ SDLK_BACKSPACE, Input::Keyboard::Key::Backspace },
				{ SDLK_TAB, Input::Keyboard::Key::Tab },
				{ SDLK_RETURN, Input::Keyboard::Key::Enter },
				{ SDLK_LSHIFT, Input::Keyboard::Key::LeftShift },
				{ SDLK_RSHIFT, Input::Keyboard::Key::RightShift },
				{ SDLK_LCTRL, Input::Keyboard::Key::LeftCtrl },
				{ SDLK_RCTRL, Input::Keyboard::Key::RightCtrl },
				{ SDLK_LALT, Input::Keyboard::Key::LeftAlt },
				{ SDLK_RALT, Input::Keyboard::Key::RightAlt },
				{ SDLK_PAUSE, Input::Keyboard::Key::Pause },
				{ SDLK_CAPSLOCK, Input::Keyboard::Key::CapsLock },
				{ SDLK_ESCAPE, Input::Keyboard::Key::Escape },
				{ SDLK_SPACE, Input::Keyboard::Key::Space },
				{ SDLK_PAGEUP, Input::Keyboard::Key::PageUp },
				{ SDLK_PAGEDOWN, Input::Keyboard::Key::PageDown },
				{ SDLK_END, Input::Keyboard::Key::End },
				{ SDLK_HOME, Input::Keyboard::Key::Home },
				{ SDLK_LEFT, Input::Keyboard::Key::Left },
				{ SDLK_UP, Input::Keyboard::Key::Up },
				{ SDLK_RIGHT, Input::Keyboard::Key::Right },
				{ SDLK_DOWN, Input::Keyboard::Key::Down },
				{ SDLK_PRINTSCREEN, Input::Keyboard::Key::PrintScreen },
				{ SDLK_INSERT, Input::Keyboard::Key::Insert },
				{ SDLK_DELETE, Input::Keyboard::Key::Delete },

				{ SDLK_a, Input::Keyboard::Key::A },
				{ SDLK_b, Input::Keyboard::Key::B },
				{ SDLK_c, Input::Keyboard::Key::C },
				{ SDLK_d, Input::Keyboard::Key::D },
				{ SDLK_e, Input::Keyboard::Key::E },
				{ SDLK_f, Input::Keyboard::Key::F },
				{ SDLK_g, Input::Keyboard::Key::G },
				{ SDLK_h, Input::Keyboard::Key::H },
				{ SDLK_i, Input::Keyboard::Key::I },
				{ SDLK_j, Input::Keyboard::Key::J },
				{ SDLK_k, Input::Keyboard::Key::K },
				{ SDLK_l, Input::Keyboard::Key::L },
				{ SDLK_m, Input::Keyboard::Key::M },
				{ SDLK_n, Input::Keyboard::Key::N },
				{ SDLK_o, Input::Keyboard::Key::O },
				{ SDLK_p, Input::Keyboard::Key::P },
				{ SDLK_q, Input::Keyboard::Key::Q },
				{ SDLK_r, Input::Keyboard::Key::R },
				{ SDLK_s, Input::Keyboard::Key::S },
				{ SDLK_t, Input::Keyboard::Key::T },
				{ SDLK_u, Input::Keyboard::Key::U },
				{ SDLK_v, Input::Keyboard::Key::V },
				{ SDLK_w, Input::Keyboard::Key::W },
				{ SDLK_x, Input::Keyboard::Key::X },
				{ SDLK_y, Input::Keyboard::Key::Y },
				{ SDLK_z, Input::Keyboard::Key::Z },

				{ SDLK_KP_0, Input::Keyboard::Key::NumPad0 },
				{ SDLK_KP_1, Input::Keyboard::Key::NumPad1 },
				{ SDLK_KP_2, Input::Keyboard::Key::NumPad2 },
				{ SDLK_KP_3, Input::Keyboard::Key::NumPad3 },
				{ SDLK_KP_4, Input::Keyboard::Key::NumPad4 },
				{ SDLK_KP_5, Input::Keyboard::Key::NumPad5 },
				{ SDLK_KP_6, Input::Keyboard::Key::NumPad6 },
				{ SDLK_KP_7, Input::Keyboard::Key::NumPad7 },
				{ SDLK_KP_8, Input::Keyboard::Key::NumPad8 },
				{ SDLK_KP_9, Input::Keyboard::Key::NumPad9 },

				{ SDLK_KP_MULTIPLY, Input::Keyboard::Key::Multiply },
				{ SDLK_KP_PLUS, Input::Keyboard::Key::Add },
				{ SDLK_KP_MINUS, Input::Keyboard::Key::Subtract },
				{ SDLK_KP_DECIMAL, Input::Keyboard::Key::Decimal },
				{ SDLK_KP_DIVIDE, Input::Keyboard::Key::Divide },

				{ SDLK_F1, Input::Keyboard::Key::F1 },
				{ SDLK_F2, Input::Keyboard::Key::F2 },
				{ SDLK_F3, Input::Keyboard::Key::F3 },
				{ SDLK_F4, Input::Keyboard::Key::F4 },
				{ SDLK_F5, Input::Keyboard::Key::F5 },
				{ SDLK_F6, Input::Keyboard::Key::F6 },
				{ SDLK_F7, Input::Keyboard::Key::F7 },
				{ SDLK_F8, Input::Keyboard::Key::F8 },
				{ SDLK_F9, Input::Keyboard::Key::F9 },
				{ SDLK_F10, Input::Keyboard::Key::F10 },
				{ SDLK_F11, Input::Keyboard::Key::F11 },
				{ SDLK_F12, Input::Keyboard::Key::F12 },

				{ SDLK_BACKQUOTE, Input::Keyboard::Key::Tilde },
			};

			static const std::unordered_map<int, Input::Keyboard::Event::Type> TypeMap = {
				{ SDL_KEYDOWN, Input::Keyboard::Event::Type::Pressed },
				{ SDL_KEYUP, Input::Keyboard::Event::Type::Released }
			};

			if (!KeyMap.contains(event.key.keysym.sym))
				continue;

			sky::Emit(Input::Keyboard::Event{
				.type = TypeMap.at(event.type),
				.key = KeyMap.at(event.key.keysym.sym)
			});
		}
		else if (event.type == SDL_JOYDEVICEADDED)
		{
			auto id = event.jdevice.which;
			if (SDL_IsGameController(id)) {
				auto gc = SDL_GameControllerOpen(id);
				if (gc)
				{
					auto id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
					gGamepads[id] = gc;
					sky::Emit(Input::Joystick::ChangedEvent{
						.id = id,
						.type = Input::Joystick::ChangedEvent::Type::Connected
					});
				}
			}
		}
		else if (event.type == SDL_JOYDEVICEREMOVED)
		{
			auto id = event.jdevice.which;
			if (gGamepads.contains(id))
			{
				SDL_GameControllerClose(gGamepads.at(id));
				gGamepads.erase(id);
				sky::Emit(Input::Joystick::ChangedEvent{
					.id = id,
					.type = Input::Joystick::ChangedEvent::Type::Disconnected
				});
			}
		}
	}

	int width;
	int height;
	SDL_GetWindowSize(gWindow, &width, &height);

	width *= mScale;
	height *= mScale;

	if (mWidth != width || mHeight != height)
	{
		mWidth = width;
		mHeight = height;
		sky::Emit(ResizeEvent({ width, height }));
	}
}

void SystemEmscripten::quit()
{
}

bool SystemEmscripten::isFinished() const
{
	return false;
}

bool SystemEmscripten::isKeyPressed(Input::Keyboard::Key key) const
{
	static const std::unordered_map<Input::Keyboard::Key, SDL_Keycode> KeyMap = {
		{ Input::Keyboard::Key::Backspace, SDLK_BACKSPACE },
		{ Input::Keyboard::Key::Tab, SDLK_TAB },
		{ Input::Keyboard::Key::Enter, SDLK_RETURN },
		{ Input::Keyboard::Key::LeftShift, SDLK_LSHIFT },
		{ Input::Keyboard::Key::RightShift, SDLK_RSHIFT },
		{ Input::Keyboard::Key::LeftCtrl, SDLK_LCTRL },
		{ Input::Keyboard::Key::RightCtrl, SDLK_RCTRL },
		{ Input::Keyboard::Key::LeftAlt, SDLK_LALT },
		{ Input::Keyboard::Key::RightAlt, SDLK_RALT },
		{ Input::Keyboard::Key::Pause, SDLK_PAUSE },
		{ Input::Keyboard::Key::CapsLock, SDLK_CAPSLOCK },
		{ Input::Keyboard::Key::Escape, SDLK_ESCAPE },
		{ Input::Keyboard::Key::Space, SDLK_SPACE },
		{ Input::Keyboard::Key::PageUp, SDLK_PAGEUP },
		{ Input::Keyboard::Key::PageDown, SDLK_PAGEDOWN },
		{ Input::Keyboard::Key::End, SDLK_END },
		{ Input::Keyboard::Key::Home, SDLK_HOME },
		{ Input::Keyboard::Key::Left, SDLK_LEFT },
		{ Input::Keyboard::Key::Up, SDLK_UP },
		{ Input::Keyboard::Key::Right, SDLK_RIGHT },
		{ Input::Keyboard::Key::Down, SDLK_DOWN },
		{ Input::Keyboard::Key::PrintScreen, SDLK_PRINTSCREEN },
		{ Input::Keyboard::Key::Insert, SDLK_INSERT },
		{ Input::Keyboard::Key::Delete, SDLK_DELETE },

		{ Input::Keyboard::Key::A, SDLK_a },
		{ Input::Keyboard::Key::B, SDLK_b },
		{ Input::Keyboard::Key::C, SDLK_c },
		{ Input::Keyboard::Key::D, SDLK_d },
		{ Input::Keyboard::Key::E, SDLK_e },
		{ Input::Keyboard::Key::F, SDLK_f },
		{ Input::Keyboard::Key::G, SDLK_g },
		{ Input::Keyboard::Key::H, SDLK_h },
		{ Input::Keyboard::Key::I, SDLK_i },
		{ Input::Keyboard::Key::J, SDLK_j },
		{ Input::Keyboard::Key::K, SDLK_k },
		{ Input::Keyboard::Key::L, SDLK_l },
		{ Input::Keyboard::Key::M, SDLK_m },
		{ Input::Keyboard::Key::N, SDLK_n },
		{ Input::Keyboard::Key::O, SDLK_o },
		{ Input::Keyboard::Key::P, SDLK_p },
		{ Input::Keyboard::Key::Q, SDLK_q },
		{ Input::Keyboard::Key::R, SDLK_r },
		{ Input::Keyboard::Key::S, SDLK_s },
		{ Input::Keyboard::Key::T, SDLK_t },
		{ Input::Keyboard::Key::U, SDLK_u },
		{ Input::Keyboard::Key::V, SDLK_v },
		{ Input::Keyboard::Key::W, SDLK_w },
		{ Input::Keyboard::Key::X, SDLK_x },
		{ Input::Keyboard::Key::Y, SDLK_y },
		{ Input::Keyboard::Key::Z, SDLK_z },

		{ Input::Keyboard::Key::NumPad0, SDLK_KP_0 },
		{ Input::Keyboard::Key::NumPad1, SDLK_KP_1 },
		{ Input::Keyboard::Key::NumPad2, SDLK_KP_2 },
		{ Input::Keyboard::Key::NumPad3, SDLK_KP_3 },
		{ Input::Keyboard::Key::NumPad4, SDLK_KP_4 },
		{ Input::Keyboard::Key::NumPad5, SDLK_KP_5 },
		{ Input::Keyboard::Key::NumPad6, SDLK_KP_6 },
		{ Input::Keyboard::Key::NumPad7, SDLK_KP_7 },
		{ Input::Keyboard::Key::NumPad8, SDLK_KP_8 },
		{ Input::Keyboard::Key::NumPad9, SDLK_KP_9 },

		{ Input::Keyboard::Key::Multiply, SDLK_KP_MULTIPLY },
		{ Input::Keyboard::Key::Add, SDLK_KP_PLUS },
		{ Input::Keyboard::Key::Subtract, SDLK_KP_MINUS },
		{ Input::Keyboard::Key::Decimal, SDLK_KP_DECIMAL },
		{ Input::Keyboard::Key::Divide, SDLK_KP_DIVIDE },

		{ Input::Keyboard::Key::F1, SDLK_F1 },
		{ Input::Keyboard::Key::F2, SDLK_F2 },
		{ Input::Keyboard::Key::F3, SDLK_F3 },
		{ Input::Keyboard::Key::F4, SDLK_F4 },
		{ Input::Keyboard::Key::F5, SDLK_F5 },
		{ Input::Keyboard::Key::F6, SDLK_F6 },
		{ Input::Keyboard::Key::F7, SDLK_F7 },
		{ Input::Keyboard::Key::F8, SDLK_F8 },
		{ Input::Keyboard::Key::F9, SDLK_F9 },
		{ Input::Keyboard::Key::F10, SDLK_F10 },
		{ Input::Keyboard::Key::F11, SDLK_F11 },
		{ Input::Keyboard::Key::F12, SDLK_F12 },

		{ Input::Keyboard::Key::Tilde, SDLK_BACKQUOTE },
	};

	if (!KeyMap.contains(key))
		return false;

	auto* state = SDL_GetKeyboardState(NULL);

	return state[SDL_GetScancodeFromKey(KeyMap.at(key))] > 0;
}

bool SystemEmscripten::isKeyPressed(Input::Mouse::Button key) const
{
	static const std::unordered_map<Input::Mouse::Button, int> ButtonMap = {
		{ Input::Mouse::Button::Left, SDL_BUTTON_LEFT },
		{ Input::Mouse::Button::Middle, SDL_BUTTON_MIDDLE },
		{ Input::Mouse::Button::Right, SDL_BUTTON_RIGHT },
	};

	if (!ButtonMap.contains(key))
		return false;

	auto state = SDL_GetMouseState(NULL, NULL);

	return state & SDL_BUTTON(ButtonMap.at(key));
}

void SystemEmscripten::resize(int width, int height)
{
}

void SystemEmscripten::setTitle(const std::string& text)
{
}

void SystemEmscripten::hideCursor()
{
}

void SystemEmscripten::showCursor()
{
}

void SystemEmscripten::setCursorPos(int x, int y)
{
}

std::optional<glm::ivec2> SystemEmscripten::getCursorPos() const
{
	return mCursorPos;
}

std::string SystemEmscripten::getAppName() const
{
	return mAppName;
}

std::string SystemEmscripten::getUUID() const
{
	return std::to_string(glm::linearRand(1, 10000000)); // TODO
}

void SystemEmscripten::initializeBilling(const ProductsMap& products)
{
}

void SystemEmscripten::purchase(const std::string& product)
{
}

void SystemEmscripten::alert(const std::string& text)
{
}

void* SystemEmscripten::getNativeWindowHandle() const
{
	return gWindow;
}

std::string SystemEmscripten::getClipboardText() const
{
	return std::string(SDL_GetClipboardText());
}

void SystemEmscripten::setClipboardText(const std::string& text)
{
	SDL_SetClipboardText(text.c_str());
}

const std::vector<std::string>& SystemEmscripten::getArguments() const
{
	return gArguments;
}

void SystemEmscripten::updateGamepadMapping(const char* str)
{
	std::istringstream stream(str);
	std::string line;
	while (std::getline(stream, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (!line.empty() && line[0] != '#')
			SDL_GameControllerAddMapping(line.c_str());
	}
}

bool SystemEmscripten::isJoystickPresent(int index) const
{
	return gGamepads.contains(index);
}

const unsigned char* SystemEmscripten::getJoystickButtons(int jid, int* count) const
{
	static unsigned char g_buttonBuffer[32] = { 0 };

	auto joy = SDL_GameControllerGetJoystick(gGamepads.at(jid));
	auto num = SDL_JoystickNumButtons(joy);

	if (num > 32)
		num = 32;

	for (int i = 0; i < num; ++i)
	{
		g_buttonBuffer[i] = (unsigned char)SDL_JoystickGetButton(joy, i);
	}

	if (count)
		*count = num;

	return g_buttonBuffer;
}

const float* SystemEmscripten::getJoystickAxes(int jid, int* count) const
{
	static float g_axisBuffer[32] = { 0.0f };

	auto joy = SDL_GameControllerGetJoystick(gGamepads.at(jid));
	auto numAxes = SDL_JoystickNumAxes(joy);

	if (numAxes > 32)
		numAxes = 32;

	for (int i = 0; i < numAxes; ++i)
	{
		auto value = SDL_JoystickGetAxis(joy, i);
		g_axisBuffer[i] = value / 32768.0f;
	}

	if (count)
		*count = numAxes;

	return g_axisBuffer;
}

bool SystemEmscripten::getGamepadState(int jid, Input::Joystick::GamepadState* state) const
{
    if (!state)
        return false;

	if (!gGamepads.contains(jid))
        return false;

    auto gc = gGamepads.at(jid);

    const SDL_GameControllerButton btns[] = {
        SDL_CONTROLLER_BUTTON_A,
        SDL_CONTROLLER_BUTTON_B,
        SDL_CONTROLLER_BUTTON_X,
        SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_START,
        SDL_CONTROLLER_BUTTON_GUIDE,
        SDL_CONTROLLER_BUTTON_LEFTSTICK,
        SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_DPAD_UP,
        SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT
    };

    for (int i = 0; i < 15; ++i)
	{
        state->buttons[i] = (unsigned char)SDL_GameControllerGetButton(gc, btns[i]);
    }

    state->axes[0] = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
    state->axes[1] = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
    state->axes[2] = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
    state->axes[3] = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;
    state->axes[4] = (SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERLEFT) - (32768.0f * 0.5f)) / (32768.0f * 0.5f);
    state->axes[5] = (SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) - (32768.0f * 0.5f)) / (32768.0f * 0.5f);

    return true;
}

#endif
