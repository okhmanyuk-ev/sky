#include "timestep_fixer.h"
#include <common/frame_system.h>

using namespace Common;

void TimestepFixer::execute(sky::Duration dTime, Callback callback)
{
	if (!mEnabled)
	{
		callback(dTime);
		return;
	}

	if (mSkipLongFrames)
	{
		if (dTime > mLongFrameDuration)
			dTime = mLongFrameDuration;
	}

	mTimeAccumulator += dTime;

	while (mTimeAccumulator >= mTimestep)
	{
		callback(mTimestep);
		mTimeAccumulator -= mTimestep;
	}

	if (mForceTimeCompletion)
	{
		if (mTimeAccumulator > sky::Duration::zero())
		{
			callback(mTimeAccumulator);
			mTimeAccumulator = sky::Duration::zero();
		}
	}
}

void TimestepFixer::execute(Callback callback)
{
	execute(FRAME->getTimeDelta(), callback);
}
