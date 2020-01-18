#pragma once

#include <Core/engine.h>

#include <Common/frame_system.h>
#include <Console/system.h>
#include <Platform/system.h>

#include <glm/glm.hpp>

namespace Shared
{
	class ConsoleHelperCommands : public Common::FrameSystem::Frameable
	{
	public:
		ConsoleHelperCommands();
		~ConsoleHelperCommands();

	private:
		void frame() override;

	private:
		bool mWantShowCVars = false;
		bool mWantImguiDemo = false;
		
	};
}