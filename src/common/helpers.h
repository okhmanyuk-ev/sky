#pragma once

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <nlohmann/json.hpp>
#include <platform/asset.h>
#include <core/clock.h>

namespace Common::Helpers
{
	std::string SecondsToFmtString(int seconds);
	bool Chance(float normalized_percent);
	std::string BytesToNiceString(uint64_t value);
	std::string BytesArrayToNiceString(void* mem, size_t size);

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

	uint32_t crc32(void* data, size_t size, uint32_t initial = 0);

	nlohmann::json LoadJsonFromAsset(const Platform::Asset& asset);
	nlohmann::json LoadBsonFromAsset(const Platform::Asset& asset);

	float SmoothValueAssign(float src, float dst, Clock::Duration dTime, float friction = 0.1f);
	glm::vec2 SmoothValueAssign(const glm::vec2& src, const glm::vec2& dst, Clock::Duration dTime, float friction = 0.1f);
	glm::vec3 SmoothValueAssign(const glm::vec3& src, const glm::vec3& dst, Clock::Duration dTime, float friction = 0.1f);
	float SmoothRotationAssign(float src_radians, float dst_radians, Clock::Duration dTime, float friction = 0.1f);
}
