#include "performance_console_commands.h"

#include <common/console_commands.h>

#include <imgui.h>
#include <shared/imgui_user.h>
#include <renderer/defines.h>
#include <shared/stats_system.h>
#include <network/system.h>
#include <common/helpers.h>
#include <graphics/system.h>

using namespace Shared;

PerformanceConsoleCommands::PerformanceConsoleCommands()
{
	CONSOLE->registerCVar("hud_show_fps", "show fps on screen", { "int" },
		CVAR_GETTER_INT(mWantShowFps),
		CVAR_SETTER_INT(mWantShowFps));

	CONSOLE->registerCVar("hud_show_drawcalls", "show drawcalls statistics", { "int" },
		CVAR_GETTER_INT(mWantShowDrawcalls),
		CVAR_SETTER_INT(mWantShowDrawcalls));

	CONSOLE->registerCVar("hud_show_batches", "show batches statistics", { "int" },
		CVAR_GETTER_INT(mWantShowBatches),
		CVAR_SETTER_INT(mWantShowBatches));

	CONSOLE->registerCVar("hud_show_tasks", "show tasks count on screen", { "int" },
		CVAR_GETTER_INT(mWantShowTasks),
		CVAR_SETTER_INT(mWantShowTasks));

	CONSOLE->registerCVar("hud_show_net_speed", "show net speed", { "bool" },
		CVAR_GETTER_BOOL(mWantShowNetSpeed), CVAR_SETTER_BOOL(mWantShowNetSpeed));

	CONSOLE->registerCVar("hud_show_net_pps", "show net packets per second", { "bool" },
		CVAR_GETTER_BOOL(mWantShowNetPps), CVAR_SETTER_BOOL(mWantShowNetPps));
}

PerformanceConsoleCommands::~PerformanceConsoleCommands()
{
	//
}

void PerformanceConsoleCommands::onFrame()
{
	if (mWantShowFps > 1)
		ENGINE_STATS("fps", std::to_string(mFramerateCounter.getFramerate()) + " (" + std::to_string(mFramerateCounter.getAverageFramerate()) + " avg)");
	else if (mWantShowFps > 0)
		ENGINE_STATS("fps", mFramerateCounter.getFramerate());

	if (mWantShowDrawcalls > 0)
		ENGINE_STATS("drawcalls", RENDERER->getDrawcalls());

	if (mWantShowBatches > 0)
		ENGINE_STATS("batches", GRAPHICS->getBatchesCount());

	if (mWantShowTasks > 1)
		ENGINE_STATS("tasks", std::to_string(TASK->getTasksCount()) + " at " + std::to_string(TASK->getThreadsCount()) + " threads");
	else if (mWantShowTasks > 0)
		ENGINE_STATS("tasks", TASK->getTasksCount());

	if (mWantShowNetSpeed)
		STATS_INDICATE_GROUP("net", "net speed", Common::Helpers::BytesToNiceString(NETWORK->getBytesPerSecond()) + "/s");

	if (mWantShowNetPps)
		STATS_INDICATE_GROUP("net", "net pps", std::to_string(NETWORK->getPacketsPerSecond()));
}