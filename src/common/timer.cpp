#include "timer.h"

using namespace Common;

void Timer::frame()
{
	mPassed += FRAME->getTimeDelta();

	if (mPassed < mInterval)
		return;

	mPassed = Clock::Duration::zero();

	auto callback = mCallback;

	if (callback)
	{
		callback();
	}
}