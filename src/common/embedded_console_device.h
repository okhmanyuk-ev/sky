#pragma once

#include <console/device.h>

namespace Common
{
	class EmbeddedConsoleDevice : public Console::Device
	{
	public:
		typedef std::function<void(const std::string&, Console::Color)> WriteCallback;
		typedef std::function<void()> ClearCallback;

	public:
		EmbeddedConsoleDevice();
		~EmbeddedConsoleDevice();

	public:
		void write(const std::string& s, Console::Color color = Console::Color::Default) override;
		void writeLine(const std::string& s, Console::Color color = Console::Color::Default) override;
		void clear() override;

		bool isOpened() const  override { return true; }

		bool isEnabled() const override { return true; }
		void setEnabled(bool value)  override { }

	public:
		void setWriteCallback(WriteCallback value) { mWriteCallback = value; }
		void setWriteLineCallback(WriteCallback value) { mWriteLineCallback = value; }
		void setClearCallback(ClearCallback value) { mClearCallback = value; }

	private:
		WriteCallback mWriteCallback = nullptr;
		WriteCallback mWriteLineCallback = nullptr;
		ClearCallback mClearCallback = nullptr;
	};
}