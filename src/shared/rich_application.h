#pragma once

#include <Core/engine.h>

#include <Common/task_system.h>
#include <Common/frame_system.h>
#include <Common/event_system.h>

#include <Shared/console_device.h>
#include <Console/system.h>

#include <Common/console_commands.h>
#include <Shared/graphical_console_commands.h>
#include <Shared/performance_console_commands.h>
#include <Shared/console_helper_commands.h>
#include <Shared/cache_system.h>

#include <Shared/imgui_system.h>

#include <Platform/system.h>
#include <Renderer/system.h>

#include <Graphics/system.h>
#include <Audio/system.h>
#include <vector>
#include <Shared/renderer_debug.h>
#include <Renderer/system_d3d11.h>
#include <Renderer/system_gl.h>

#include <Shared/graphical_application.h>

namespace Shared
{
	class RichApplication : public GraphicalApplication
	{
	public:
		RichApplication(const std::string& appname);

	protected:
		void postFrame() override;
		void postImguiPresent() override;

	public:
		struct LoadingTask
		{
			std::string name;
			std::function<void()> callback;
		};

	public:
		void addLoadingTasks(const std::vector<LoadingTask>& tasks);

	public:
		bool isInitialized() const { return mInitialized; }

	private:
		std::vector<LoadingTask> mLoadingTasks;
		bool mLoading = false;
		Clock::TimePoint mStartLoadingTime;
		bool mLoaded = false;
		size_t mLoadingPos = 0;
		bool mInitialized = false;
		float mGameFade = 1.0f;
		float mLoadingFade = 1.0f;
		
	private:
		void initInternal();

	protected:
		virtual void initialize() = 0;
		virtual void frame() = 0;
		virtual void loading(const std::string& stage, float progress);
	};
}