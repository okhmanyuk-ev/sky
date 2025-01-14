#pragma once

#include <common/timer.h>
#include <common/frame_system.h>

namespace Common
{
	class FramerateCounter: public FrameSystem::Frameable
	{
	public:
		FramerateCounter();
		~FramerateCounter();

	private:
		void onFrame() override;

	public:
		auto getFrameCount() const { return mFrameCount; }
		auto getFramerate() const { return mFramerate; }
		auto getStartTime() const { return mStartTime; }
		auto getDuration() const { return sky::Now() - getStartTime(); }
		auto getAverageFramerate() const { return mAverageFramerate; }

	private:
		Common::Timer mTimer;
		uint64_t mFrameCount = 0;
		int mFramerateInternal = 0;
		int mFramerate = 0;
		int mAverageFramerate = 0;
		sky::TimePoint mStartTime = sky::Now();
	};
}