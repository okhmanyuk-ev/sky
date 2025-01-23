#include "performance_console_commands.h"

#include <common/console_commands.h>
#include <sky/renderer.h>
#include <imgui.h>
#include <shared/imgui_user.h>
#include <shared/stats_system.h>
#include <network/system.h>
#include <common/helpers.h>
#include <graphics/system.h>
#include <common/threadpool.h>

using namespace Shared;

void PerformanceConsoleCommands::onFrame()
{
	if (mWantShowFps > 0)
	{
		auto str = std::to_string(mFramerateCounter.getFramerate());

		auto time_scale = SCHEDULER->getTimeScale();

		if (time_scale != 1.0f)
			str = fmt::format("(x{}) {}", time_scale, str);

		auto now = sky::Now();

		static bool choked = false;
		static auto choke_time = now;

		if (SCHEDULER->isChoked())
		{
			choked = true;
			choke_time = now;
		}
		else if (now - choke_time > sky::FromSeconds(2.0f))
		{
			choked = false;
		}

		if (choked)
			str = "(!) " + str;

		if (mWantShowFps > 1)
		{
			auto avg_framerate = mFramerateCounter.getAverageFramerate();
			str = fmt::format("{} ({} avg)", str, avg_framerate);
		}

		ENGINE_STATS("fps", str);
	}

	if (mWantShowDrawcalls > 0)
		ENGINE_STATS("drawcalls", RENDERER->getDrawcalls());

	if (mWantShowBatches > 0)
	{
		ENGINE_STATS("batches", GRAPHICS->getBatchesCount());
		ENGINE_STATS("flushes", GRAPHICS->getBatchFlushCount());
	}

	if (mWantShowTargets > 0)
		ENGINE_STATS("targets", GRAPHICS->getRenderTargets().size());

	if (mWantShowTasks > 1)
		ENGINE_STATS("tasks", std::to_string(THREADPOOL->getTasksCount()) + " at " + std::to_string(THREADPOOL->getThreadsCount()) + " threads");
	else if (mWantShowTasks > 0)
		ENGINE_STATS("tasks", THREADPOOL->getTasksCount());

	if (mWantShowNetSpeed)
		STATS_INDICATE_GROUP("net", "net speed", Common::Helpers::BytesToNiceString(NETWORK->getBytesPerSecond()) + "/s");

	if (mWantShowNetPps)
		STATS_INDICATE_GROUP("net", "net pps", std::to_string(NETWORK->getPacketsPerSecond()));
}
