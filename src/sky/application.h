#pragma once

#include <platform/system.h>
#include <graphics/system.h>
#include <network/system.h>
#include <common/console_commands.h>
#include <shared/graphical_console_commands.h>
#include <shared/performance_console_commands.h>
#include <shared/console_helper_commands.h>
#include <shared/stats_system.h>
#include <shared/touch_emulator.h>
#include <shared/gesture_detector.h>
#include <common/profiler_system.h>
#include <shared/scene_stylebook.h>
#include <shared/scene_editor.h>

namespace sky
{
	class Application final
	{
	public:
		enum class Flag
		{
			Audio,
			Network,
			Scene
		};

		using Flags = std::set<Flag>;

	public:
		Application(const std::string& appname, const Flags& flags = {});
		~Application();

	public:
		void run();

	private:
		Flags mFlags;

	public:
		auto getScene() const { return mScene; }
		const auto& getStartupKeyValues() const { return mStartupKeyValues; }

	private: // Scene
		std::shared_ptr<Scene::Scene> mScene;
		std::shared_ptr<Shared::SceneEditor> mSceneEditor;
		std::unordered_map<std::string, std::string> mStartupKeyValues;

	private:
		std::shared_ptr<Common::ConsoleCommands> mConsoleCommands;
		std::shared_ptr<Shared::GraphicalConsoleCommands> mGraphicalConsoleCommands;

		std::shared_ptr<Shared::PerformanceConsoleCommands> mPerformanceConsoleCommands;
		std::shared_ptr<Shared::ConsoleHelperCommands> mConsoleHelperCommands;

		std::shared_ptr<Shared::TouchEmulator> mTouchEmulator;
		std::shared_ptr<Shared::GestureDetector> mGestureDetector;
	};
}