#include "helpers.h"

using namespace Shared;

std::string Helpers::SecondsToFmtString(int seconds)
{
	int h = seconds / 60 / 60;
	int m = (seconds / 60) % 60;
	int s = seconds % 60;

	return fmt::format("{:02}:{:02}:{:02}", h, m, s);
}

bool Helpers::Chance(float normalized_percent)
{
	return glm::linearRand(0.0f, 1.0f) <= normalized_percent;
}
