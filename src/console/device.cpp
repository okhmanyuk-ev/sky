#include "device.h"
#include <fmt/format.h>

void sky::Log(const std::string& text)
{
	if (!ENGINE->hasSystem<Console::Device>())
		return;

	CONSOLE_DEVICE->writeLine(text);
}

void sky::Log(Console::Color color, const std::string& text)
{
	if (!ENGINE->hasSystem<Console::Device>())
		return;

	CONSOLE_DEVICE->writeLine(text, color);
}
