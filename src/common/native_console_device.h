#ifdef _WIN32
#pragma once

#include <Console/device.h>
#include <Common/event_system.h>
#include <Common/frame_system.h>

#include <thread>
#include <Windows.h>

namespace Common
{
	class NativeConsoleDevice : public Console::Device
	{
	public:
		NativeConsoleDevice();
		~NativeConsoleDevice();

	public:
		void write(const std::string& s, Console::Color color = Console::Color::Default) override;
		void writeLine(const std::string& s, Console::Color color = Console::Color::Default) override;
		void clear() override;
		bool isOpened() const override { return true; }

		bool isEnabled() const override { return true; }
		void setEnabled(bool value) override {};

	public:
		void setTitle(const std::string& s);

	private:
		std::thread mReadThread;
		HANDLE mConsoleHandle;
	};
}
#endif