#include "scheduler.h"
#include <thread>
#include <algorithm>

using namespace Common;

void Scheduler::add(StatusCallback callback)
{
	mFramers.push_back(callback);
}

void Scheduler::addInfinity(Callback callback)
{
	add([callback] {
		callback();
		return Status::Continue;
	});
}

void Scheduler::addOne(Callback callback)
{
	add([callback] {
		callback();
		return Status::Finished;
	});
}

void Scheduler::addOneThreadsafe(Callback callback)
{
	mMutex.lock();
	mThreadsafeCallbacks.push_back(callback);
	mMutex.unlock();
}

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

	auto it = mFramers.begin();
	while (it != mFramers.end())
	{
		if ((*it)() == Status::Continue)
			++it;
		else
			it = mFramers.erase(it);
	}

	mMutex.lock();
	for (auto callback : mThreadsafeCallbacks)
	{
		callback();
	}
	mThreadsafeCallbacks.clear();
	mMutex.unlock();
}

Scheduler::Frameable::Frameable()
{
	SCHEDULER->add([this, finished = mFinished] {
		if (*finished)
			return Status::Finished;

		onFrame();
		return Status::Continue;
	});
}

Scheduler::Frameable::~Frameable()
{
	*mFinished = true;
}

Scheduler::Framer::Framer(Callback callback) : Frameable(),
	mCallback(callback)
{
}

void Scheduler::Framer::onFrame()
{
	if (mCallback == nullptr)
		return;

	mCallback();
}