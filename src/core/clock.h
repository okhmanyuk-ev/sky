#pragma once

#include <chrono>

namespace Clock
{
	typedef std::chrono::high_resolution_clock::duration Duration;
	typedef std::chrono::high_resolution_clock::time_point TimePoint;

	inline auto Now()
	{
		return std::chrono::high_resolution_clock::now();
	}

	inline auto SystemNow()
	{
		return std::chrono::system_clock::now();
	}

	inline auto SystemNowSeconds()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(SystemNow().time_since_epoch()).count();
	}

	inline uint32_t ToMilliseconds(Duration value)
	{
		return std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(value).count();
	}

	inline Duration FromMilliseconds(int value)
	{
		return std::chrono::duration_cast<Duration>(std::chrono::duration<int, std::chrono::milliseconds::period>(value));
	}

	template <typename T = float> T ToSeconds(Duration value)
	{
		return std::chrono::duration<T, std::chrono::seconds::period>(value).count();
	}

	template <typename T> Duration FromSeconds(T value)
	{
		return std::chrono::duration_cast<Duration>(std::chrono::duration<T, std::chrono::seconds::period>(value));
	}
}