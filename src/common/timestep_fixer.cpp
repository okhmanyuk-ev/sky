#include "timestep_fixer.h"
#include <core/clock.h>
#include <common/frame_system.h>

using namespace Common;

void TimestepFixer::execute(Clock::Duration dTime)
{
	mTimeAccumulator += dTime;

	while (mTimeAccumulator >= mTimestep)
	{
		mCallback(mTimestep);
		mTimeAccumulator -= mTimestep;
	}

	if (mForceTimeCompletion)
	{
		if (mTimeAccumulator > Clock::Duration::zero())
		{
			mCallback(mTimeAccumulator);
			mTimeAccumulator = Clock::Duration::zero();
		}
	}
}

void TimestepFixer::execute()
{
	execute(FRAME->getTimeDelta());
}
