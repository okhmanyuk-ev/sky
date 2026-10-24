#include "key_bindings.h"
#include <ranges>
#include <optional>
#include <algorithm>
#include <imgui.h>
#include <sky/console.h>
#include <sky/utils.h>

using namespace Shared;
using namespace Platform::Input;

static const std::map<Keyboard::Key, std::string> KeyNames = {
	{ Keyboard::Key::Backspace, "backspace" },
	{ Keyboard::Key::Tab, "tab" },
	{ Keyboard::Key::Enter, "enter" },
	{ Keyboard::Key::Pause, "pause" },
	{ Keyboard::Key::CapsLock, "capslock" },
	{ Keyboard::Key::Escape, "escape" },
	{ Keyboard::Key::Space, "space" },
	{ Keyboard::Key::PageUp, "pgup" },
	{ Keyboard::Key::PageDown, "pgdn" },
	{ Keyboard::Key::End, "end" },
	{ Keyboard::Key::Home, "home" },
	{ Keyboard::Key::Left, "leftarrow" },
	{ Keyboard::Key::Up, "uparrow" },
	{ Keyboard::Key::Right, "rightarrow" },
	{ Keyboard::Key::Down, "downarrow" },
	{ Keyboard::Key::PrintScreen, "printscreen" },
	{ Keyboard::Key::Insert, "ins" },
	{ Keyboard::Key::Delete, "del" },
	{ Keyboard::Key::D0, "0" },
	{ Keyboard::Key::D1, "1" },
	{ Keyboard::Key::D2, "2" },
	{ Keyboard::Key::D3, "3" },
	{ Keyboard::Key::D4, "4" },
	{ Keyboard::Key::D5, "5" },
	{ Keyboard::Key::D6, "6" },
	{ Keyboard::Key::D7, "7" },
	{ Keyboard::Key::D8, "8" },
	{ Keyboard::Key::D9, "9" },
	{ Keyboard::Key::A, "a" },
	{ Keyboard::Key::B, "b" },
	{ Keyboard::Key::C, "c" },
	{ Keyboard::Key::D, "d" },
	{ Keyboard::Key::E, "e" },
	{ Keyboard::Key::F, "f" },
	{ Keyboard::Key::G, "g" },
	{ Keyboard::Key::H, "h" },
	{ Keyboard::Key::I, "i" },
	{ Keyboard::Key::J, "j" },
	{ Keyboard::Key::K, "k" },
	{ Keyboard::Key::L, "l" },
	{ Keyboard::Key::M, "m" },
	{ Keyboard::Key::N, "n" },
	{ Keyboard::Key::O, "o" },
	{ Keyboard::Key::P, "p" },
	{ Keyboard::Key::Q, "q" },
	{ Keyboard::Key::R, "r" },
	{ Keyboard::Key::S, "s" },
	{ Keyboard::Key::T, "t" },
	{ Keyboard::Key::U, "u" },
	{ Keyboard::Key::V, "v" },
	{ Keyboard::Key::W, "w" },
	{ Keyboard::Key::X, "x" },
	{ Keyboard::Key::Y, "y" },
	{ Keyboard::Key::Z, "z" },
	{ Keyboard::Key::LeftWindows, "lwin" },
	{ Keyboard::Key::RightWindows, "rwin" },
	{ Keyboard::Key::Apps, "apps" },
	{ Keyboard::Key::NumPad0, "kp_ins" },
	{ Keyboard::Key::NumPad1, "kp_end" },
	{ Keyboard::Key::NumPad2, "kp_downarrow" },
	{ Keyboard::Key::NumPad3, "kp_pgdn" },
	{ Keyboard::Key::NumPad4, "kp_leftarrow" },
	{ Keyboard::Key::NumPad5, "kp_5" },
	{ Keyboard::Key::NumPad6, "kp_rightarrow" },
	{ Keyboard::Key::NumPad7, "kp_home" },
	{ Keyboard::Key::NumPad8, "kp_uparrow" },
	{ Keyboard::Key::NumPad9, "kp_pgup" },
	{ Keyboard::Key::Multiply, "kp_multiply" },
	{ Keyboard::Key::Add, "kp_plus" },
	{ Keyboard::Key::Subtract, "kp_minus" },
	{ Keyboard::Key::Decimal, "kp_del" },
	{ Keyboard::Key::Divide, "kp_slash" },
	{ Keyboard::Key::F1, "f1" },
	{ Keyboard::Key::F2, "f2" },
	{ Keyboard::Key::F3, "f3" },
	{ Keyboard::Key::F4, "f4" },
	{ Keyboard::Key::F5, "f5" },
	{ Keyboard::Key::F6, "f6" },
	{ Keyboard::Key::F7, "f7" },
	{ Keyboard::Key::F8, "f8" },
	{ Keyboard::Key::F9, "f9" },
	{ Keyboard::Key::F10, "f10" },
	{ Keyboard::Key::F11, "f11" },
	{ Keyboard::Key::F12, "f12" },
	{ Keyboard::Key::LeftShift, "shift" },
	{ Keyboard::Key::RightShift, "rshift" },
	{ Keyboard::Key::LeftCtrl, "ctrl" },
	{ Keyboard::Key::RightCtrl, "rctrl" },
	{ Keyboard::Key::LeftAlt, "alt" },
	{ Keyboard::Key::RightAlt, "ralt" },
	{ Keyboard::Key::Semicolon, "semicolon" },
	{ Keyboard::Key::Plus, "=" },
	{ Keyboard::Key::Comma, "," },
	{ Keyboard::Key::Minus, "-" },
	{ Keyboard::Key::Period, "." },
	{ Keyboard::Key::Question, "/" },
//	{ Keyboard::Key::Tilde, "`" },
	{ Keyboard::Key::OpenBrackets, "[" },
	{ Keyboard::Key::Pipe, "\\" },
	{ Keyboard::Key::CloseBrackets, "]" },
	{ Keyboard::Key::Quotes, "apostrophe" },
};

static const std::map<Mouse::Button, std::string> MouseButtonNames = {
	{ Mouse::Button::Left, "mouse1" },
	{ Mouse::Button::Right, "mouse2" },
	{ Mouse::Button::Middle, "mouse3" },
};

static const std::string MouseWheelUp = "mwheelup";
static const std::string MouseWheelDown = "mwheeldown";

template <typename T>
static std::optional<std::string> FindKeyName(const std::map<T, std::string>& names, T value)
{
	if (!names.contains(value))
		return std::nullopt;

	return names.at(value);
}

static bool IsKeyNameValid(const std::string& key)
{
	auto has_name = [&](const auto& names) {
		return std::ranges::contains(names | std::views::values, key);
	};

	return has_name(KeyNames) || has_name(MouseButtonNames) || key == MouseWheelUp || key == MouseWheelDown;
}

static std::optional<std::string> ParseKeyName(const std::string& arg)
{
	auto key = sky::to_lower(arg);

	if (IsKeyNameValid(key))
		return key;

	sky::Log("\"{}\" isn't a valid key", key);
	return std::nullopt;
}

static bool IsInputBlocked()
{
	return sky::GetService<sky::Console>()->isOpened() || ImGui::GetIO().WantTextInput;
}

static bool IsMouseInputBlocked()
{
	return IsInputBlocked() || ImGui::GetIO().WantCaptureMouse;
}

KeyBindings::KeyBindings()
{
	sky::GetService<sky::CommandProcessor>()->addItem("bind", sky::CommandProcessor::Command("bind a key to a command",
		{ "key" }, {}, { "command" }, [this](const std::vector<std::string>& args) {
			auto key = ParseKeyName(args.at(0));

			if (!key.has_value())
				return;

			if (args.size() > 1)
				bind(key.value(), sky::join(args | std::views::drop(1) | std::ranges::to<std::vector>(), " "));
			else if (mBindings.contains(key.value()))
				sky::Log("\"{}\" = \"{}\"", key.value(), mBindings.at(key.value()));
			else
				sky::Log("\"{}\" is not bound", key.value());
		}));

	sky::AddCommand("unbind", "remove a key binding", { "key" }, {}, {}, [this](std::string arg) {
		if (auto key = ParseKeyName(arg); key.has_value())
			unbind(key.value());
	});

	sky::AddCommand("unbindall", "remove all key bindings", {}, {}, {}, [this] {
		unbindAll();
	});

	sky::AddCommand("bindlist", "show list of key bindings", {}, {}, {}, [this] {
		if (mBindings.empty())
			sky::Log("bind list is empty");

		for (const auto& [key, command] : mBindings)
			sky::Log(" - \"{}\" = \"{}\"", key, command);
	});
}

KeyBindings::~KeyBindings()
{
	if (!sky::Locator<sky::CommandProcessor>::Exists())
		return;

	for (auto name : { "bind", "unbind", "unbindall", "bindlist" })
		sky::GetService<sky::CommandProcessor>()->removeItem(name);
}

void KeyBindings::onEvent(const Keyboard::Event& e)
{
	auto key = FindKeyName(KeyNames, e.key);

	if (!key.has_value())
		return;

	if (e.type == Keyboard::Event::Type::Pressed && !IsInputBlocked())
		keyDown(key.value());
	else if (e.type == Keyboard::Event::Type::Released)
		keyUp(key.value());
}

void KeyBindings::onEvent(const Mouse::ButtonEvent& e)
{
	auto key = FindKeyName(MouseButtonNames, e.button);

	if (!key.has_value())
		return;

	if (e.type == Mouse::ButtonEvent::Type::Pressed && !IsMouseInputBlocked())
		keyDown(key.value());
	else if (e.type == Mouse::ButtonEvent::Type::Released)
		keyUp(key.value());
}

void KeyBindings::onEvent(const Mouse::ScrollEvent& e)
{
	if (e.scroll.y == 0.0f || IsMouseInputBlocked())
		return;

	const auto& key = e.scroll.y > 0.0f ? MouseWheelUp : MouseWheelDown;
	keyDown(key);
	keyUp(key);
}

void KeyBindings::keyDown(const std::string& key)
{
	if (!mBindings.contains(key) || mPendingReleases.contains(key))
		return;

	auto command = mBindings.at(key);

	if (command.starts_with('+'))
		mPendingReleases.insert({ key, "-" + command.substr(1) });

	sky::GetService<sky::CommandProcessor>()->execute(command);
}

void KeyBindings::keyUp(const std::string& key)
{
	auto release = mPendingReleases.extract(key);

	if (!release.empty())
		sky::GetService<sky::CommandProcessor>()->execute(release.mapped());
}
