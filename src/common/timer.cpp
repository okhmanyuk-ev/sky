#include "timer.h"

using namespace Common;

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