#pragma once

#include <common/task_system.h>
#include <platform/system.h>
#include <common/frame_system.h>
#include <common/framerate_counter.h>
#include <common/task_system.h>
#include <sky/console.h>
#include <deque>

namespace Shared
{
	class PerformanceConsoleCommands : public Common::FrameSystem::Frameable
	{
	private:
		void onFrame() override;

	private:
		Common::FramerateCounter mFramerateCounter;

	private:
		sky::CVarInt mWantShowFps = sky::CVarInt("hud_show_fps", 0, "show fps on screen");
		sky::CVarInt mWantShowDrawcalls = sky::CVarInt("hud_show_drawcalls", 0, "show drawcalls statistics");
		sky::CVarInt mWantShowBatches = sky::CVarInt("hud_show_batches", 0, "show batches statistics");
		sky::CVarInt mWantShowTargets = sky::CVarInt("hud_show_targets", 0, "show render targets statistics");
		sky::CVarInt mWantShowTasks = sky::CVarInt("hud_show_tasks", 0, "show tasks count on screen");
		sky::CVarBool mWantShowNetSpeed = sky::CVarBool("hud_show_net_speed", false);
		sky::CVarBool mWantShowNetPps = sky::CVarBool("hud_show_net_pps", false);

	private:
		std::deque<float> mDeltaTimes;
	};
}