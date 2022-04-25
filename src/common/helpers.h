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

	inline const float DefaultFriction = 0.1f;

	float SmoothValueAssign(float src, float dst, Clock::Duration dTime, float friction = DefaultFriction);
	glm::vec2 SmoothValueAssign(const glm::vec2& src, const glm::vec2& dst, Clock::Duration dTime, float friction = DefaultFriction);
	glm::vec3 SmoothValueAssign(const glm::vec3& src, const glm::vec3& dst, Clock::Duration dTime, float friction = DefaultFriction);
	glm::mat4 SmoothValueAssign(const glm::mat4& src, const glm::mat4& dst, Clock::Duration dTime, float friction = DefaultFriction);
	float SmoothRotationAssign(float src_radians, float dst_radians, Clock::Duration dTime, float friction = DefaultFriction);

	// TODO: find identical function in glm
	// TODO: it seems every smoothstep function call should be changed to this
	template<typename T>
	inline T invLerp(T edge0, T edge1, T x) 
	{
		return (x - edge0) / (edge1 - edge0);
	};
}
