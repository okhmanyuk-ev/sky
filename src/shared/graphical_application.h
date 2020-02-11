#pragma once

#include <Core/engine.h>

#include <Common/task_system.h>
#include <Common/frame_system.h>
#include <Common/event_system.h>
#include <Shared/imgui_system.h>
#include <Platform/system.h>
#include <Renderer/system.h>
#include <Graphics/system.h>
#include <Network/system.h>
#include <Common/console_commands.h>
#include <Shared/graphical_console_commands.h>
#include <Shared/performance_console_commands.h>
#include <Shared/console_helper_commands.h>
#include <Shared/console_device.h>
#include <Shared/localization_system.h>
#include <Shared/stats_system.h>
#include <Shared/cache_system.h>
#include <Renderer/system_d3d11.h>
#include <Renderer/system_gl.h>
#include <Shared/touch_emulator.h>
#include <Shared/gesture_detector.h>

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
		std::shared_ptr<Shared::ImguiSystem> mImguiSystem;

		std::shared_ptr<Common::ConsoleCommands> mConsoleCommands;
		std::shared_ptr<Shared::GraphicalConsoleCommands> mGraphicalConsoleCommands;

		std::shared_ptr<Shared::PerformanceConsoleCommands> mPerformanceConsoleCommands;
		std::shared_ptr<Shared::ConsoleHelperCommands> mConsoleHelperCommands;

		std::shared_ptr<Shared::TouchEmulator> mTouchEmulator;
		std::shared_ptr<Shared::GestureDetector> mGestureDetector;
	};
}