#include "task.h"
#include <sky/clock.h>
#include <sky/scheduler.h>

sky::Task<> sky::Tasks::NextFrame()
{
	co_await std::suspend_always{};
}

sky::Task<> sky::Tasks::WaitForSeconds(float seconds)
{
	while (seconds > 0.0f)
	{
		seconds -= sky::ToSeconds(sky::Scheduler::Instance->getTimeDelta());
		co_await NextFrame();
	}
}

sky::Task<> sky::Tasks::WaitForFrames(int count)
{
	for (int i = 0; i < count; i++)
	{
		co_await NextFrame();
	}
}

sky::Task<> sky::Tasks::WaitWhile(std::function<bool()> condition)
{
	while (condition())
	{
		co_await NextFrame();
	}
}
