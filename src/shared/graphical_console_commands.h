#pragma once

#include <common/frame_system.h>

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
