#pragma once

#include <common/scheduler.h>
#include <sky/console.h>
#include <skygfx/skygfx.h>
#include <graphics/system.h>
#include <platform/system.h>

namespace Shared
{
	class GraphicalConsoleCommands : public Common::Scheduler::Frameable
	{
	public:
		GraphicalConsoleCommands();

	private:
		void onFrame() override;

	private:
		sky::CVar<bool> mShowTargets = sky::CVar<bool>("r_showtargets", false);

		sky::CVar<bool> mVsync = sky::CVar<bool>("r_vsync", skygfx::IsVsyncEnabled, skygfx::SetVsync);

		sky::CVar<bool> mBatching = sky::CVar<bool>("r_batching",
			std::bind(&Graphics::System::isBatching, GRAPHICS),
			std::bind(&Graphics::System::setBatching, GRAPHICS, std::placeholders::_1));

		sky::CVar<float> mScale = sky::CVar<float>("r_scale",
			std::bind(&Platform::System::getScale, PLATFORM),
			std::bind(&Platform::System::setScale, PLATFORM, std::placeholders::_1),
			"logical scaling on retina displays");
	};
}
