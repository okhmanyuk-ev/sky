#pragma once

#include <string>
#include <functional>
#include <core/engine.h>
#include <fmt/format.h>

#define LOG(TEXT) sky::console::Log(TEXT)
#define LOGF(TEXT, ...) sky::console::Log(TEXT, __VA_ARGS__)
#define LOGC(TEXT, COLOR) sky::console::Log(COLOR, TEXT)
#define LOGCF(TEXT, COLOR, ...) sky::console::Log(TEXT, COLOR, __VA_ARGS__)

#define CONSOLE_DEVICE ENGINE->getSystem<Console::Device>()

namespace Console
{
	enum class Color
	{
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkCyan = 3,
		DarkRed = 4,
		DarkMagenta = 5,
		DarkYellow = 6,
		Gray = 7,
		DarkGray = 8,
		Blue = 9,
		Green = 10,
		Cyan = 11,
		Red = 12,
		Magenta = 13,
		Yellow = 14,
		White = 15,
		Default
	};

	class Device
	{
	public:
		struct ReadEvent 
		{
			std::string text;
		};

	public:
		virtual void write(const std::string& s, Color color = Color::Default) = 0;
		virtual void writeLine(const std::string& s, Color color = Color::Default) = 0;
		virtual void clear() = 0;

		virtual bool isOpened() const = 0;

		virtual bool isEnabled() const = 0;
		virtual void setEnabled(bool value) = 0;
	};
}

namespace sky::console
{
	void Log(const std::string& text);
	void Log(Console::Color color, const std::string& text);

	template<typename... Args>
	void Log(const std::string& text, Args&&... args)
	{
		Log(fmt::format(text, args...));
	}

	template<typename... Args>
	void Log(Console::Color color, const std::string& text, Args&&... args)
	{
		Log(color, fmt::format(text, args...));
	}
}