#pragma once

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Common::Helpers
{
	std::string SecondsToFmtString(int seconds);
	bool Chance(float normalized_percent);
	std::string BytesToNiceString(uint64_t value);

	inline const float RadialLimit = glm::pi<float>() * 2.0f;

	inline float RadialToGlobal(float radius, float radial_pos)
	{
		return radius * radial_pos;
	}

	inline float GlobalToRadial(float radius, float global_pos)
	{
		return global_pos / radius;
	}

	inline float GetPerimeter(float radius)
	{
		return RadialToGlobal(radius, RadialLimit);
	}
}
