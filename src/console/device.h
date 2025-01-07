#pragma once

#include <string>
#include <functional>
#include <fmt/format.h>
#include <sky/singleton.h>

#define CONSOLE_DEVICE sky::Singleton<Console::Device>::GetInstance()

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

namespace sky
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