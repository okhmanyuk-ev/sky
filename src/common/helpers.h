#pragma once

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <nlohmann/json.hpp>
#include <platform/asset.h>
#include <core/clock.h>
#include <common/frame_system.h>

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

	constexpr float DefaultFriction = 0.1f;
	constexpr float DefaultDeltaLimit = 1.0f / 30.0f;

	template<class T>
	T SmoothValue(T src, T dst, Clock::Duration dTime, float friction = DefaultFriction, float delta_limit = DefaultDeltaLimit)
	{
		auto distance = dst - src;
		auto delta = glm::min(delta_limit, Clock::ToSeconds(dTime)) * 100.0f;
		return src + (distance * delta * friction);
	}

	template<class T>
	T SmoothValue(T src, T dst, float friction = DefaultFriction, float delta_limit = DefaultDeltaLimit)
	{
		return SmoothValue(src, dst, FRAME->getTimeDelta(), friction, delta_limit);
	}

	float SmoothRotation(float src_radians, float dst_radians, Clock::Duration dTime, float friction = DefaultFriction,
		float delta_limit = DefaultDeltaLimit);

	float SmoothRotation(float src_radians, float dst_radians, float friction = DefaultFriction,
		float delta_limit = DefaultDeltaLimit);

	// TODO: find identical function in glm
	// TODO: it seems every smoothstep function call should be changed to this
	template<typename T>
	inline T invLerp(T edge0, T edge1, T x) 
	{
		return (x - edge0) / (edge1 - edge0);
	};
}
