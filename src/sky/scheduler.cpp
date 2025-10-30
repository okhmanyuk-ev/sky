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
	mTasksCount = mTasks.size();

	auto tasks = std::move(mTasks);

	auto it = tasks.begin();
	while (it != tasks.end())
	{
		it->resume();
		if (it->is_completed())
			it = tasks.erase(it);
		else
			++it;
	}

	mTasks.insert(mTasks.begin(), std::make_move_iterator(tasks.begin()), std::make_move_iterator(tasks.end()));
}

void Scheduler::run(Task<>&& task)
{
	mTasks.push_back(std::move(task));
}
