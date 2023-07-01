#include "device.h"
#include <fmt/format.h>

void sky::Log(const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text);
}

void sky::Log(Console::Color color, const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text, color);
}
