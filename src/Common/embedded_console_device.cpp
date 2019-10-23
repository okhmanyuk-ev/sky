#include "embedded_console_device.h"

using namespace Common;

EmbeddedConsoleDevice::EmbeddedConsoleDevice()
{
	//
}

EmbeddedConsoleDevice::~EmbeddedConsoleDevice()
{
	//
}

void EmbeddedConsoleDevice::write(const std::string& s, Console::Color color)
{
	if (mWriteCallback) 
	{
		mWriteCallback(s, color);
	}
}

void EmbeddedConsoleDevice::writeLine(const std::string& s, Console::Color color)
{
	if (mWriteLineCallback)
	{
		mWriteLineCallback(s, color);
	}
}

void EmbeddedConsoleDevice::clear()
{
	if (mClearCallback)
	{
		mClearCallback();
	}
}