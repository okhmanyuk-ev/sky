#pragma once

#include <Core/engine.h>

#include <Common/task_system.h>
#include <Common/frame_system.h>
#include <Common/event_system.h>

#include <Shared/imgui_system.h>

#include <Platform/system.h>
#include <Renderer/system.h>

#include <Graphics/system.h>

#include <Common/console_commands.h>
#include <Shared/graphical_console_commands.h>
#include <Shared/performance_console_commands.h>
#include <Shared/console_helper_commands.h>
#include <Shared/console_device.h>
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
	public:
		GraphicalApplication(const std::string& appname);
		
	public:
		void run();

	protected:
		virtual void preFrame() {};
		virtual void postFrame() {};

		virtual void postImguiPresent() {};

	private:
		Core::Engine mEngine;

		std::shared_ptr<Common::TaskSystem> mTaskSystem;
		std::shared_ptr<Common::FrameSystem> mFrame;
		std::shared_ptr<Common::EventSystem> mEvent;
		
		std::shared_ptr<Platform::System> mPlatform;
		std::shared_ptr<RendererDebug<Renderer::SystemCrossplatform>> mRenderer;

		std::shared_ptr<Shared::ConsoleDevice> mConsoleDevice;
		std::shared_ptr<Console::System> mConsole;

		std::shared_ptr<Graphics::System> mGraphics;
		std::shared_ptr<Shared::ImguiSystem> mImguiSystem;

	private:
		std::shared_ptr<Common::ConsoleCommands> mConsoleCommands;
		std::shared_ptr<Shared::GraphicalConsoleCommands> mGraphicalConsoleCommands;

		std::shared_ptr<Shared::PerformanceConsoleCommands> mPerformanceConsoleCommands;
		std::shared_ptr<Shared::ConsoleHelperCommands> mConsoleHelperCommands;

		std::shared_ptr<Shared::StatsSystem> mStats;
		std::shared_ptr<Shared::CacheSystem> mCache;
		std::shared_ptr<Shared::TouchEmulator> mTouchEmulator;
		std::shared_ptr<Shared::GestureDetector> mGestureDetector;
	};
}