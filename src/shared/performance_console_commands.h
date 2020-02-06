#pragma once

#include <Core/engine.h>
#include <Console/system.h>
#include <Common/task_system.h>
#include <Platform/system.h>
#include <Console/system.h>
#include <Common/frame_system.h>
#include <Common/framerate_counter.h>
#include <Common/task_system.h>
#include <deque>
#include <Shared/drawcall_counter.h>

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