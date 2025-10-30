#include "performance_console_commands.h"

#include <common/console_commands.h>
#include <sky/renderer.h>
#include <imgui.h>
#include <shared/imgui_user.h>
#include <shared/stats_system.h>
#include <network/system.h>
#include <common/helpers.h>
#include <graphics/system.h>
#include <sky/threadpool.h>

using namespace Shared;

void PerformanceConsoleCommands::onFrame()
{
	if (mWantShowFps > 0)
	{
		auto str = std::to_string(mFramerateCounter.getFramerate());

		auto time_scale = sky::Scheduler::Instance->getTimeScale();

		if (time_scale != 1.0f)
			str = fmt::format("(x{}) {}", time_scale, str);

		auto now = sky::Now();

		static bool choked = false;
		static auto choke_time = now;

		if (sky::Scheduler::Instance->isChoked())
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

		sky::Indicator("engine", "fps", str);
	}

	if (mWantShowDrawcalls > 0)
		sky::Indicator("engine", "drawcalls", RENDERER->getDrawcalls());

	if (mWantShowBatches > 0)
	{
		sky::Indicator("engine", "batches", GRAPHICS->getBatchesCount());
		sky::Indicator("engine", "flushes", GRAPHICS->getBatchFlushCount());
	}

	if (mWantShowTargets > 0)
		sky::Indicator("engine", "targets", GRAPHICS->getRenderTargets().size());

	if (mWantShowThreadpool > 1)
		sky::Indicator("engine", "threadpool", std::to_string(THREADPOOL->getTasksCount()) + " at " + std::to_string(THREADPOOL->getThreadsCount()) + " threads");
	else if (mWantShowThreadpool > 0)
		sky::Indicator("engine", "threadpool", THREADPOOL->getTasksCount());

#ifndef EMSCRIPTEN
	if (mWantShowNetSpeed)
		sky::Indicator("net", "net speed", Common::Helpers::BytesToNiceString(NETWORK->getBytesPerSecond()) + "/s");

	if (mWantShowNetPps)
		sky::Indicator("net", "net pps", std::to_string(NETWORK->getPacketsPerSecond()));
#endif
}
