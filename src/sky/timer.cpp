#include "timer.h"

using namespace sky;

void Timer::onFrame()
{
	mPassed += SCHEDULER->getTimeDelta();

	if (mPassed < mInterval)
		return;

	mPassed = sky::Duration::zero();

	auto callback = mCallback;

	if (callback)
	{
		callback();
	}
}