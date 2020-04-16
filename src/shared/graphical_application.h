#pragma once

#include <core/engine.h>

#include <common/task_system.h>
#include <common/frame_system.h>
#include <common/event_system.h>
#include <shared/imgui_system.h>
#include <platform/system.h>
#include <renderer/system.h>
#include <graphics/system.h>
#include <network/system.h>
#include <common/console_commands.h>
#include <shared/graphical_console_commands.h>
#include <shared/performance_console_commands.h>
#include <shared/console_helper_commands.h>
#include <shared/console_device.h>
#include <shared/localization_system.h>
#include <shared/stats_system.h>
#include <shared/cache_system.h>
#include <renderer/system_d3d11.h>
#include <renderer/system_gl.h>
#include <shared/touch_emulator.h>
#include <shared/gesture_detector.h>
#include <common/profiler_system.h>

namespace Shared
{
	class GraphicalApplication
	{
	private:
		static inline Core::Engine Engine;

	public:
		GraphicalApplication(const std::string& appname);
		~GraphicalApplication();

	public:
		void run();

	protected:
		virtual void preFrame() {};
		virtual void postFrame() {};

		virtual void postImguiPresent() {};
		
	private:
		std::shared_ptr<Common::ConsoleCommands> mConsoleCommands;
		std::shared_ptr<Shared::GraphicalConsoleCommands> mGraphicalConsoleCommands;

		std::shared_ptr<Shared::PerformanceConsoleCommands> mPerformanceConsoleCommands;
		std::shared_ptr<Shared::ConsoleHelperCommands> mConsoleHelperCommands;

		std::shared_ptr<Shared::TouchEmulator> mTouchEmulator;
		std::shared_ptr<Shared::GestureDetector> mGestureDetector;
	};
}