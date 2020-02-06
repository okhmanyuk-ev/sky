#include "interpolator.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Common;

void Interpolator::frame()
{
	mPassed += FRAME->getTimeDelta();

	if (mPassed >= mDuration)
	{
		mValue = mDestinationValue;
		mPassed = mDuration;
		auto processCallback = mProcessCallback;
		auto finishCallback = mFinishCallback;
		mProcessCallback = nullptr;
		mFinishCallback = nullptr;
		if (processCallback) { processCallback(mValue); }
		if (finishCallback) { finishCallback(); }
		return;
	}

	if (mEasingFunction == nullptr)
	{
		mValue = mStartValue;
	}
	else
	{
		float p = Clock::ToSeconds(mPassed);
		float d = Clock::ToSeconds(mDuration);

		mValue = glm::lerp(mStartValue, mDestinationValue, mEasingFunction(p / d));
	}

	if (mProcessCallback) 
		mProcessCallback(mValue);
}