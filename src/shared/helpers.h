#pragma once

#include <fmt/format.h>

namespace Shared::Helpers
{
	std::string SecondsToFmtString(int seconds);
	bool Chance(float normalized_percent);
}
