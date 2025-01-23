#pragma once

#include <common/task_system.h>
#include <platform/system.h>
#include <common/scheduler.h>
#include <common/framerate_counter.h>
#include <common/task_system.h>
#include <sky/console.h>
#include <deque>

namespace Shared
{
	class PerformanceConsoleCommands : public Common::Scheduler::Frameable
	{
	private:
		void onFrame() override;

	private:
		Common::FramerateCounter mFramerateCounter;

	private:
		sky::CVar<int> mWantShowFps = sky::CVar<int>("hud_show_fps", 0, "show fps on screen");
		sky::CVar<int> mWantShowDrawcalls = sky::CVar<int>("hud_show_drawcalls", 0, "show drawcalls statistics");
		sky::CVar<int> mWantShowBatches = sky::CVar<int>("hud_show_batches", 0, "show batches statistics");
		sky::CVar<int> mWantShowTargets = sky::CVar<int>("hud_show_targets", 0, "show render targets statistics");
		sky::CVar<int> mWantShowTasks = sky::CVar<int>("hud_show_tasks", 0, "show tasks count on screen");
		sky::CVar<bool> mWantShowNetSpeed = sky::CVar<bool>("hud_show_net_speed", false);
		sky::CVar<bool> mWantShowNetPps = sky::CVar<bool>("hud_show_net_pps", false);

	private:
		std::deque<float> mDeltaTimes;
	};
}