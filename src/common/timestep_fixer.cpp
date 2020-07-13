#include "timestep_fixer.h"
#include <core/clock.h>
#include <common/frame_system.h>

using namespace Common;

void TimestepFixer::execute(float dTime)
{
	mTimeAccumulator += dTime;

	while (mTimeAccumulator >= mTimestep)
	{
		mCallback(mTimestep);
		mTimeAccumulator -= mTimestep;
	}

	if (mForceTimeCompletion)
	{
		if (mTimeAccumulator > 0.0f)
		{
			mCallback(mTimeAccumulator);
			mTimeAccumulator = 0.0f;
		}
	}
}

void TimestepFixer::execute()
{
	execute(Clock::ToSeconds(FRAME->getTimeDelta()));
}
