#pragma once
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <nlohmann/json.hpp>
#include <sky/asset.h>
#include <sky/scheduler.h>

namespace Common::Helpers
{
	std::string SecondsToFmtString(int seconds);
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

	nlohmann::json LoadJsonFromAsset(const sky::Asset& asset);
	nlohmann::json LoadBsonFromAsset(const sky::Asset& asset);

	// TODO: find identical function in glm
	// TODO: it seems every smoothstep function call should be changed to this
	template<typename T>
	inline T invLerp(T edge0, T edge1, T x)
	{
		return (x - edge0) / (edge1 - edge0);
	};
}

namespace sky
{
	float sanitize(float value, float default_value = 0.0f);
	glm::vec2 sanitize(glm::vec2 value, float default_value = 0.0f);

	constexpr float default_friction = 0.1f;
	constexpr float default_delta_limit = 1.0f / 30.0f;

	template<class T>
	T ease_towards(T src, T dst, sky::Duration dTime = Scheduler::Instance->getTimeDelta(),
		float friction = default_friction, float delta_limit = default_delta_limit)
	{
		auto distance = dst - src;
		auto delta = glm::min(delta_limit, sky::ToSeconds(dTime)) * 100.0f;
		return src + (distance * delta * friction);
	}

	float ease_rotation_towards(float src_radians, float dst_radians, sky::Duration dTime = Scheduler::Instance->getTimeDelta(),
		float friction = default_friction, float delta_limit = default_delta_limit);

	bool chance(float normalized_percent);
}
