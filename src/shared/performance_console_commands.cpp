#include "performance_console_commands.h"

#include <common/console_commands.h>

#include <imgui.h>
#include <shared/imgui_user.h>
#include <shared/stats_system.h>
#include <network/system.h>
#include <common/helpers.h>
#include <graphics/system.h>

using namespace Shared;

PerformanceConsoleCommands::PerformanceConsoleCommands()
{
	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_fps", "show fps on screen", { "int (1/2)" },
		CVAR_GETTER_INT(mWantShowFps),
		CVAR_SETTER_INT(mWantShowFps));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_drawcalls", "show drawcalls statistics", { "int" },
		CVAR_GETTER_INT(mWantShowDrawcalls),
		CVAR_SETTER_INT(mWantShowDrawcalls));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_batches", "show batches statistics", { "int" },
		CVAR_GETTER_INT(mWantShowBatches),
		CVAR_SETTER_INT(mWantShowBatches));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_targets", "show render targets statistics", { "int" },
		CVAR_GETTER_INT(mWantShowTargets),
		CVAR_SETTER_INT(mWantShowTargets));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_tasks", "show tasks count on screen", { "int" },
		CVAR_GETTER_INT(mWantShowTasks),
		CVAR_SETTER_INT(mWantShowTasks));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_net_speed", "show net speed", { "bool" },
		CVAR_GETTER_BOOL(mWantShowNetSpeed), CVAR_SETTER_BOOL(mWantShowNetSpeed));

	sky::GetService<sky::CommandProcessor>()->registerCVar("hud_show_net_pps", "show net packets per second", { "bool" },
		CVAR_GETTER_BOOL(mWantShowNetPps), CVAR_SETTER_BOOL(mWantShowNetPps));
}

PerformanceConsoleCommands::~PerformanceConsoleCommands()
{
}

void PerformanceConsoleCommands::onFrame()
{
	if (mWantShowFps > 0)
	{
		auto str = std::to_string(mFramerateCounter.getFramerate());

		auto time_scale = FRAME->getTimeScale();

		if (time_scale != 1.0f)
			str = fmt::format("(x{}) {}", time_scale, str);

		auto now = sky::Now();

		static bool choked = false;
		static auto choke_time = now;

		if (FRAME->isChoked())
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
		ENGINE_STATS("tasks", std::to_string(TASK->getTasksCount()) + " at " + std::to_string(TASK->getThreadsCount()) + " threads");
	else if (mWantShowTasks > 0)
		ENGINE_STATS("tasks", TASK->getTasksCount());

	if (mWantShowNetSpeed)
		STATS_INDICATE_GROUP("net", "net speed", Common::Helpers::BytesToNiceString(NETWORK->getBytesPerSecond()) + "/s");

	if (mWantShowNetPps)
		STATS_INDICATE_GROUP("net", "net pps", std::to_string(NETWORK->getPacketsPerSecond()));
}
