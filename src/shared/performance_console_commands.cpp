#include "performance_console_commands.h"

#include <common/console_commands.h>

#include <imgui.h>
#include <shared/imgui_user.h>
#include <renderer/defines.h>
#include <shared/stats_system.h>

using namespace Shared;

PerformanceConsoleCommands::PerformanceConsoleCommands()
{
	CONSOLE->registerCVar("hud_show_fps", "show fps on screen", { "int" },
		CVAR_GETTER_INT(mWantShowFps),
		CVAR_SETTER_INT(mWantShowFps));

	CONSOLE->registerCVar("hud_show_drawcalls", "show drawcalls statistics", { "int" },
		CVAR_GETTER_INT(mWantShowDrawcalls),
		CVAR_SETTER_INT(mWantShowDrawcalls));

	CONSOLE->registerCVar("hud_show_tasks", "show tasks count on screen", { "int" },
		CVAR_GETTER_INT(mWantShowTasks),
		CVAR_SETTER_INT(mWantShowTasks));

	return;

	CONSOLE->registerCVar("hud_show_framegraph", "show frame graph on screen", { "int" },
		CVAR_GETTER_INT(mWantShowFrameGraph),
		CVAR_SETTER_INT(mWantShowFrameGraph));

	CONSOLE->registerCVar("hud_framegraph_capacity", "delta graph capacity", { "int" },
		CVAR_GETTER_INT(mFrameGraphCapacity),
		CVAR_SETTER_INT(mFrameGraphCapacity));
}

PerformanceConsoleCommands::~PerformanceConsoleCommands()
{
	//
}

void PerformanceConsoleCommands::frame()
{
	if (mWantShowFps > 1)
		ENGINE_STATS("fps", std::to_string(mFramerateCounter.getFramerate()) + " (" + std::to_string(mFramerateCounter.getAverageFramerate()) + " avg)");
	else if (mWantShowFps > 0)
		ENGINE_STATS("fps", mFramerateCounter.getFramerate());

	if (mWantShowDrawcalls > 0)
		ENGINE_STATS("draw", mDrawCallCounter.getDrawCalls());

	if (mWantShowTasks > 1)
		ENGINE_STATS("tasks", std::to_string(TASK->getTasksCount()) + " at " + std::to_string(TASK->getThreadsCount()) + " threads");
	else if (mWantShowTasks > 0)
		ENGINE_STATS("tasks", TASK->getTasksCount());

	return;

	if (mWantShowFrameGraph)
	{
		mDeltaTimes.push_front(Clock::ToSeconds(FRAME->getTimeDelta()));

		while (mDeltaTimes.size() > static_cast<size_t>(mFrameGraphCapacity))
			mDeltaTimes.pop_back();

		auto getter = [](auto data, auto idx) {
			return ((std::deque<float>*)data)->at(idx);
		};

		if (mWantShowFrameGraph > 1)
		{
			float min = 1000.0;
			float max = 0.0;
			for (float delta : mDeltaTimes)
			{
				min = glm::min(delta, min);
				max = glm::max(delta, max);
			}

			auto s = "min: " + std::to_string(min) + ", max: " + std::to_string(max);
			ImGui::PlotLines("Delta", getter, &mDeltaTimes, (int)mDeltaTimes.size(), 0, s.c_str(), FLT_MAX, FLT_MAX, ImVec2(0, 64));
		}
		else
		{
			ImGui::PlotLines("Delta", getter, &mDeltaTimes, (int)mDeltaTimes.size());
		}
	}
}