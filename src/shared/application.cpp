#include "application.h"

#include <imgui.h>
#include <shared/imgui_user.h>
#include <imscene/imscene.h>
#include <audio/system.h>
#include <shared/scene_manager.h>

using namespace Shared;

Application::Application(const std::string& appname, const Flags& flags) : mFlags(flags)
{
	std::srand((unsigned int)std::time(nullptr));

	ENGINE->addSystem<Common::Event::System>(std::make_shared<Common::Event::System>());
	ENGINE->addSystem<Common::TaskSystem>(std::make_shared<Common::TaskSystem>());
	ENGINE->addSystem<Common::FrameSystem>(std::make_shared<Common::FrameSystem>());
	ENGINE->addSystem<Common::ProfilerSystem>(std::make_shared<Common::ProfilerSystem>());
	ENGINE->addSystem<Platform::System>(Platform::System::create(appname));
	ENGINE->addSystem<Renderer::System>(std::make_shared<Renderer::SystemCrossplatform>());
	ENGINE->addSystem<Console::Device>(std::make_shared<Shared::ConsoleDevice>());
	ENGINE->addSystem<Console::System>(std::make_shared<Console::System>());
	ENGINE->addSystem<Graphics::System>(std::make_shared<Graphics::System>());
	if (flags.count(Flag::Network))
	{
		ENGINE->addSystem<Network::System>(std::make_shared<Network::System>());
	}
	ENGINE->addSystem<Shared::LocalizationSystem>(std::make_shared<Shared::LocalizationSystem>());
	ENGINE->addSystem<Shared::StatsSystem>(std::make_shared<Shared::StatsSystem>());
	ENGINE->addSystem<Shared::CacheSystem>(std::make_shared<Shared::CacheSystem>());
	ENGINE->addSystem<Shared::ImguiSystem>(std::make_shared<Shared::ImguiSystem>());
	ENGINE->addSystem<Shared::Stylebook>(std::make_shared<Shared::Stylebook>());
	ENGINE->addSystem<ImScene::ImScene>(std::make_shared<ImScene::ImScene>());
	if (flags.count(Flag::Audio))
	{
		ENGINE->addSystem<Audio::System>(std::make_shared<Audio::System>());
	}

	mConsoleCommands = std::make_shared<Common::ConsoleCommands>();
	mGraphicalConsoleCommands = std::make_shared<Shared::GraphicalConsoleCommands>();
	mPerformanceConsoleCommands = std::make_shared<Shared::PerformanceConsoleCommands>();
	mConsoleHelperCommands = std::make_shared<Shared::ConsoleHelperCommands>();
	mTouchEmulator = std::make_shared<Shared::TouchEmulator>();
	mGestureDetector = std::make_shared<Shared::GestureDetector>();

	ImGui::User::SetupStyleFromColor(1.0f, 1.0f, 0.75f);

	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;

	mConsoleCommands->setQuitCallback([this] {
		PLATFORM->quit();
	});

#if !defined(PLATFORM_MOBILE)
	IMGUI_SYSTEM->setScaleIndependence(true);
#endif

	if (flags.count(Flag::Scene))
	{
		mScene = std::make_shared<Scene::Scene>();
		mScene->setInteractTestCallback([](const auto& pos) {
			return !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup) 
				&& !ImGui::IsAnyItemActive();
		});
		mSceneEditor = std::make_shared<SceneEditor>(*mScene);

		ENGINE->addSystem<Shared::SceneManager>(std::make_shared<Shared::SceneManager>());
		mScene->getRoot()->attach(SCENE_MANAGER);

		auto getter = [this] { 
			auto fps = 1.0f / Clock::ToSeconds(mScene->getTimestepFixer().getTimestep());
			return std::vector<std::string>({ std::to_string(fps) });
		};
		auto setter = [this](CON_ARGS) {
			auto sec = std::stof(CON_ARG(0));
			mScene->getTimestepFixer().setTimestep(Clock::FromSeconds(1.0f / sec));
		};
		CONSOLE->registerCVar("scene_timestep_fps", { "float" }, getter, setter);

		CONSOLE->registerCVar("scene_timestep_enabled", { "bool" }, 
			CVAR_GETTER_BOOL_FUNC(mScene->getTimestepFixer().isEnabled), 
			CVAR_SETTER_BOOL_FUNC(mScene->getTimestepFixer().setEnabled));

		CONSOLE->registerCVar("scene_timestep_force_time_completion", { "bool" },
			CVAR_GETTER_BOOL_FUNC(mScene->getTimestepFixer().getForceTimeCompletion),
			CVAR_SETTER_BOOL_FUNC(mScene->getTimestepFixer().setForceTimeCompletion));

	}
}

Application::~Application()
{
	if (mFlags.count(Flag::Scene))
	{
		ENGINE->removeSystem<Shared::SceneManager>();
		mSceneEditor = nullptr;
		mScene = nullptr;
	}
	if (mFlags.count(Flag::Audio))
	{
		ENGINE->removeSystem<Audio::System>();
	}
	ENGINE->removeSystem<ImScene::ImScene>();
	ENGINE->removeSystem<Shared::Stylebook>();
	ENGINE->removeSystem<Shared::ImguiSystem>();
	ENGINE->removeSystem<Shared::CacheSystem>();
	ENGINE->removeSystem<Shared::StatsSystem>();
	ENGINE->removeSystem<Shared::LocalizationSystem>();
	if (mFlags.count(Flag::Network))
	{
		ENGINE->removeSystem<Network::System>();
	}
	ENGINE->removeSystem<Graphics::System>();
	ENGINE->removeSystem<Console::System>();
	ENGINE->removeSystem<Console::Device>();
	ENGINE->removeSystem<Renderer::System>();
	ENGINE->removeSystem<Platform::System>();
	ENGINE->removeSystem<Common::ProfilerSystem>();
	ENGINE->removeSystem<Common::FrameSystem>();
	ENGINE->removeSystem<Common::TaskSystem>();
	//	ENGINE->removeSystem<Common::EventSystem>(); // should be removed later
}

void Application::run()
{
	while (!PLATFORM->isFinished())
	{
		PLATFORM->process();
		RENDERER->setRenderTarget(nullptr);
		RENDERER->clear();
		IMGUI_SYSTEM->begin();
		if (mFlags.count(Flag::Scene))
		{
			mScene->frame();
		}
		FRAME->frame();
		IMGUI_SYSTEM->end();
		RENDERER->present();
	}
}