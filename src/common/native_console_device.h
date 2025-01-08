#pragma once

#ifdef _WIN32

#include <console/device.h>
#include <thread>

#define NATIVE_CONSOLE_DEVICE std::static_pointer_cast<Common::NativeConsoleDevice>(CONSOLE_DEVICE)

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
		void* mConsoleHandle;
	};
}
#endif