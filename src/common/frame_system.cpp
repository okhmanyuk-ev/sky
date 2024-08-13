#include "frame_system.h"
#include <thread>
#include <algorithm>

using namespace Common;

void FrameSystem::add(StatusCallback callback)
{
	mFramers.push_back(callback);
}

void FrameSystem::addInfinity(Callback callback)
{
	add([callback] {
		callback();
		return Status::Continue;
	});
}

void FrameSystem::addOne(Callback callback)
{
	add([callback] {
		callback();
		return Status::Finished;
	});
}

void FrameSystem::addOneThreadsafe(Callback callback)
{
	mMutex.lock();
	mThreadsafeCallbacks.push_back(callback);
	mMutex.unlock();
}

void FrameSystem::frame()
{
	mFrameCount += 1;

	if (mFramerateLimit > 0)
	{
		auto frameTime = Clock::FromSeconds(1.0 / mFramerateLimit);

		while (Clock::Now() - mLastTime < frameTime)
		{
			if (!mSleepAllowed)
				continue;

			std::this_thread::sleep_for(Clock::FromMilliseconds(1));
		}
	}

	auto now = Clock::Now();

	mTimeDelta = Clock::FromSeconds(Clock::ToSeconds<double>(now - mLastTime) * mTimeScale);

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

FrameSystem::Frameable::Frameable()
{
	FRAME->add([this, finished = mFinished] {
		if (*finished)
			return Status::Finished;

		onFrame();
		return Status::Continue;
	});
}

FrameSystem::Frameable::~Frameable()
{
	*mFinished = true;
}

FrameSystem::Framer::Framer(Callback callback) : Frameable(),
	mCallback(callback)
{
}

void FrameSystem::Framer::onFrame()
{
	if (mCallback == nullptr)
		return;

	mCallback();
}