#pragma once

#include <console/system.h>
#include <common/console_commands.h>
#include <platform/system.h>
#include <graphics/system.h>

namespace Shared
{
	class GraphicalConsoleCommands : public Common::FrameSystem::Frameable
	{
	public:
		GraphicalConsoleCommands();
		~GraphicalConsoleCommands();

	private:
		void onFrame() override;

	private:
		bool mShowTargets = false;
	};
}
