#pragma once

#include <sky/scheduler.h>
#include <platform/system.h>
#include <glm/glm.hpp>
#include <sky/console.h>

namespace Shared
{
	class ConsoleHelperCommands : public sky::Scheduler::Frameable
	{
	public:
		ConsoleHelperCommands();

	private:
		void onFrame() override;

	private:
		sky::CVar<bool> mShowCVars = sky::CVar<bool>("hud_show_cvars", false, "show cvars menu on screen");
		sky::CVar<bool> mShowImguiDemo = sky::CVar<bool>("imgui_show_demo", false, "show imgui demo");
	};
}