#include "timer.h"
#include <sky/scheduler.h>

using namespace sky;

void Timer::onFrame()
{
	mPassed += sky::Scheduler::Instance->getTimeDelta();

	if (mPassed < mInterval)
		return;

	mPassed = sky::Duration::zero();

	auto callback = mCallback;

	if (callback)
	{
		callback();
	}
}