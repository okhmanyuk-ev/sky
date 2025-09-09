#include "scheduler.h"
#include <thread>
#include <algorithm>

using namespace sky;

void Scheduler::frame()
{
	mFrameCount += 1;

	if (mFramerateLimit > 0)
	{
		auto frameTime = sky::FromSeconds(1.0 / mFramerateLimit);

		while (sky::Now() - mLastTime < frameTime)
		{
			if (!mSleepAllowed)
				continue;

			std::this_thread::sleep_for(sky::FromMilliseconds(1));
		}
	}

	auto now = sky::Now();

	mTimeDelta = sky::FromSeconds(sky::ToSeconds(now - mLastTime) * mTimeScale);

	if (mTimeDeltaLimit.has_value() && mTimeDelta > mTimeDeltaLimit.value())
	{
		mTimeDelta = mTimeDeltaLimit.value();
		mChoked = true;
	}
	else
	{
		mChoked = false;
	}

	mLastTime = now;
	mUptime += mTimeDelta;

	auto it = mTasks.begin();
	while (it != mTasks.end())
	{
		it->resume();
		if (it->is_completed())
			it = mTasks.erase(it);
		else
			++it;
	}
}

void Scheduler::run(Task<>&& task)
{
	mTasks.push_back(std::move(task));
}
