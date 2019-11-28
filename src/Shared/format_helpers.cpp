#include "format_helpers.h"

using namespace Shared;

std::string FormatHelpers::SecondsToFmtString(int seconds)
{
	int h = seconds / 60 / 60;
	int m = (seconds / 60) % 60;
	int s = seconds % 60;

	return fmt::format("{:02}:{:02}:{:02}", h, m, s);
}