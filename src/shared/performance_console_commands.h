#pragma once

#include <core/engine.h>
#include <console/system.h>
#include <common/task_system.h>
#include <platform/system.h>
#include <console/system.h>
#include <common/frame_system.h>
#include <common/framerate_counter.h>
#include <common/task_system.h>
#include <deque>
#include <shared/drawcall_counter.h>

namespace Shared
{
	class PerformanceConsoleCommands : public Common::FrameSystem::Frameable
	{
	public:
		PerformanceConsoleCommands();
		~PerformanceConsoleCommands();

	private:
		void frame() override;

	private:
		Common::FramerateCounter mFramerateCounter;
		Shared::DrawCallCounter mDrawCallCounter;

	private:
		int mWantShowFps = 0;
		int mWantShowDrawcalls = 0;
		int mWantShowTasks = 0;
		int mWantShowFrameGraph = 0;
		int mFrameGraphCapacity = 240;

	private:
		std::deque<float> mDeltaTimes;
	};
}