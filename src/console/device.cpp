#include "device.h"
#include <fmt/format.h>

using namespace sky;

void console::Log(const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text);
}

void console::Log(Console::Color color, const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text, color);
}
