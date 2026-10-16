#include "scheduler.h"
#include <algorithm>

using namespace sky;

void Scheduler::frame()
{
	mFrameCount += 1;

	auto now = sky::Now();

	mTimeDelta = sky::FromSeconds(sky::ToSeconds(now - mLastTime) * mTimeScale.getValue());

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

	if (auto framerateLimit = mFramerateLimit.getValue(); framerateLimit > 0)
	{
		auto frameTime = sky::FromSeconds(1.0 / framerateLimit);

		if (now - mNextFrameTime > frameTime)
			mNextFrameTime = now;

		mNextFrameTime += frameTime;
	}

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

bool Scheduler::isFrameReady() const
{
	if (mFramerateLimit.getValue() <= 0)
		return true;

	// frame callbacks are not perfectly periodic, a slightly early one must not be skipped,
	// otherwise 30 fps limit on 60 hz display gives 20 fps
	constexpr auto jitterTolerance = std::chrono::milliseconds(2);

	return sky::Now() + jitterTolerance >= mNextFrameTime;
}

void Scheduler::run(Task<>&& task)
{
	mTasks.push_back(std::move(task));
}
