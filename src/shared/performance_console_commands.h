#pragma once

#include <console/system.h>
#include <common/task_system.h>
#include <platform/system.h>
#include <console/system.h>
#include <common/frame_system.h>
#include <common/framerate_counter.h>
#include <common/task_system.h>
#include <deque>

namespace Shared
{
	class PerformanceConsoleCommands : public Common::FrameSystem::Frameable
	{
	public:
		PerformanceConsoleCommands();
		~PerformanceConsoleCommands();

	private:
		void onFrame() override;

	private:
		Common::FramerateCounter mFramerateCounter;

	private:
		int mWantShowFps = 0;
		int mWantShowDrawcalls = 0;
		int mWantShowBatches = 0;
		int mWantShowTargets = 0;
		int mWantShowTasks = 0;
		bool mWantShowNetSpeed = false;
		bool mWantShowNetPps = false;

	private:
		std::deque<float> mDeltaTimes;
	};
}