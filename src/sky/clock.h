#pragma once

#include <chrono>

namespace sky
{
	typedef std::chrono::high_resolution_clock::duration Duration;
	typedef std::chrono::high_resolution_clock::time_point TimePoint;

	TimePoint Now();
	std::chrono::system_clock::time_point SystemNow();
	std::chrono::seconds::rep SystemNowSeconds();

	uint32_t ToMilliseconds(Duration value);
	Duration FromMilliseconds(int value);

	template <typename T = float>
	T ToSeconds(Duration value)
	{
		return std::chrono::duration<T, std::chrono::seconds::period>(value).count();
	}

	template <typename T>
	Duration FromSeconds(T value)
	{
		return std::chrono::duration_cast<Duration>(std::chrono::duration<T, std::chrono::seconds::period>(value));
	}

	template <typename T>
	Duration FromDays(T value)
	{
		return std::chrono::duration_cast<Duration>(std::chrono::duration<T, std::chrono::days::period>(value));
	}
}