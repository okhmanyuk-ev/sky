#include "clock.h"

sky::TimePoint sky::Now()
{
	return std::chrono::high_resolution_clock::now();
}

std::chrono::system_clock::time_point sky::SystemNow()
{
	return std::chrono::system_clock::now();
}

std::chrono::seconds::rep sky::SystemNowSeconds()
{
	return std::chrono::duration_cast<std::chrono::seconds>(SystemNow().time_since_epoch()).count();
}

uint32_t sky::ToMilliseconds(Duration value)
{
	return std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(value).count();
}

sky::Duration sky::FromMilliseconds(int value)
{
	return std::chrono::duration_cast<Duration>(std::chrono::duration<int, std::chrono::milliseconds::period>(value));
}
