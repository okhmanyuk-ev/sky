#pragma once

#include <common/frame_system.h>
#include <console/system.h>
#include <platform/system.h>
#include <glm/glm.hpp>

namespace Shared
{
	class ConsoleHelperCommands : public Common::FrameSystem::Frameable
	{
	public:
		ConsoleHelperCommands();
		~ConsoleHelperCommands();

	private:
		void onFrame() override;

	private:
		bool mShowCVars = false;
		bool mShowProfiler = false;
		bool mShowImguiDemo = false;
	};
}