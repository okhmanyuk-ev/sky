#include "framerate_counter.h"

using namespace Common;

FramerateCounter::FramerateCounter()
{
	mTimer.setInterval(sky::FromSeconds(1.0f));
	mTimer.setCallback([this] {
		mFramerate = mFramerateInternal;
		mFramerateInternal = 0;
		mAverageFramerate = static_cast<int>(static_cast<float>(getFrameCount()) / sky::ToSeconds(getDuration()));
	});
}

FramerateCounter::~FramerateCounter()
{
}

void FramerateCounter::onFrame()
{
	mFramerateInternal++;
	mFrameCount++;
}