#include "application.h"
#include <imgui.h>
#include <shared/imgui_user.h>
#include <shared/imscene.h>
#include <sky/audio.h>
#include <shared/scene_manager.h>
#include <shared/scene_helpers.h>
#include <regex>
#include <sky/locator.h>
#include <sky/cache.h>
#include <sky/localization.h>
#include <sky/renderer.h>
#include <sky/dispatcher.h>
#include <sky/scheduler.h>
#include <sky/threadpool.h>
#include <sky/imgui_system.h>
#include <sky/imgui_console.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace sky;

static std::unique_ptr<sky::CVar<bool>> gCVarSceneTimestepEnabled;
static std::unique_ptr<sky::CVar<bool>> gCVarSceneTimestepTimeCompletion;

Application::Application(const std::string& appname, const Flags& flags, std::optional<skygfx::BackendType> backend_type) : mFlags(flags)
{
	std::srand((unsigned int)std::time(nullptr));

	sky::Locator<sky::Dispatcher>::Init();
#ifndef EMSCRIPTEN
	sky::Locator<sky::ThreadPool>::Init();
#endif
	sky::Locator<sky::CommandProcessor>::Init();
	sky::Locator<sky::Scheduler>::Init();
	sky::Locator<Platform::System>::Set(Platform::System::create(appname));
	sky::Locator<sky::Renderer>::Init(backend_type);
	sky::Locator<sky::Console>::Set(std::make_shared<sky::ImguiConsole>());
	sky::Locator<Graphics::System>::Init();
	if (flags.count(Flag::Network))
	{
#ifndef EMSCRIPTEN
		sky::Locator<Network::System>::Init();
#endif
	}
	sky::Locator<sky::Localization>::Init();
	sky::Locator<Shared::StatsSystem>::Init();
	sky::Locator<sky::Cache>::Init();
	sky::Locator<sky::ImguiSystem>::Init();
	sky::Locator<Shared::Stylebook>::Init();
	sky::Locator<Shared::ImScene>::Init();
	if (flags.count(Flag::Audio))
	{
		sky::Locator<sky::Audio>::Init();
	}
	sky::Locator<Common::ConsoleCommands>::Init();
	sky::Locator<Shared::GraphicalConsoleCommands>::Init();
	sky::Locator<Shared::PerformanceConsoleCommands>::Init();
	sky::Locator<Shared::ConsoleHelperCommands>::Init();
	sky::Locator<Shared::TouchEmulator>::Init();
	sky::Locator<Shared::GestureDetector>::Init();

	ImGui::User::SetupStyleFromColor(1.0f, 1.0f, 0.75f);

	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;

	sky::GetService<Common::ConsoleCommands>()->setQuitCallback([this] {
		PLATFORM->quit();
	});

	IMGUI_SYSTEM->setSamplerNearest(false);

	if (flags.count(Flag::Scene))
	{
		sky::Locator<Scene::Scene>::Init();
		auto scene = sky::GetService<Scene::Scene>();

		scene->setInteractTestCallback([](const auto& pos) {
			return !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup)
				&& !ImGui::IsAnyItemActive();
		});
		sky::Locator<Shared::SceneEditor>::Init(*scene);

		sky::Locator<Shared::SceneManager>::Init();
		scene->getRoot()->attach(sky::GetService<Shared::SceneManager>());

		auto getter = [] {
			auto fps = 1.0f / sky::ToSeconds(sky::GetService<Scene::Scene>()->getTimestepFixer().getTimestep());
			return std::vector<std::string>({ std::to_string(fps) });
		};

		auto setter = [](CON_ARGS) {
			auto sec = std::stof(CON_ARG(0));
			sky::GetService<Scene::Scene>()->getTimestepFixer().setTimestep(sky::FromSeconds(1.0f / sec));
		};

		sky::AddCVar("scene_timestep_fps", sky::CommandProcessor::CVar(std::nullopt, { "float" }, getter, setter));

		gCVarSceneTimestepEnabled = std::make_unique<sky::CVar<bool>>("scene_timestep_enabled",
			std::bind(&sky::TimestepFixer::isEnabled, &scene->getTimestepFixer()),
			std::bind(&sky::TimestepFixer::setEnabled, &scene->getTimestepFixer(), std::placeholders::_1));

		gCVarSceneTimestepTimeCompletion = std::make_unique<sky::CVar<bool>>("scene_timestep_force_time_completion",
			std::bind(&sky::TimestepFixer::getForceTimeCompletion, &scene->getTimestepFixer()),
			std::bind(&sky::TimestepFixer::setForceTimeCompletion, &scene->getTimestepFixer(), std::placeholders::_1));

		sky::AddCommand("spawn_blur_glass", sky::CommandProcessor::Command(std::nullopt, {}, { { "size", "512" }, { "intensity", "0.5" }, { "passes", "1" }, { "outlined", "1" }, { "rounding", "0.0" } }, {}, [](CON_ARGS) {
			auto size = CON_ARG_FLOAT(0);
			auto intensity = CON_ARG_FLOAT(1);
			auto passes = CON_ARG_INT(2);
			auto outlined = CON_ARG_BOOL(3);
			auto rounding = CON_ARG_FLOAT(4);

			auto glass = std::make_shared<Shared::SceneHelpers::KillableByClick<Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Rounded<Scene::BlurredGlass>>>>>();
			glass->setSize(size);
			glass->setAnchor(0.5f);
			glass->setPivot(0.5f);
			glass->setOutlined(outlined);
			glass->setBlurIntensity(intensity);
			glass->setBlurPasses(passes);
			glass->setRounding(rounding);
			sky::GetService<Scene::Scene>()->getRoot()->attach(glass);
		}));

		sky::AddCommand("spawn_gray_glass", sky::CommandProcessor::Command(std::nullopt, {}, { { "size", "512" }, { "intensity", "0.5" }, { "outlined", "1" }, { "rounding", "0.0" } }, {}, [this](CON_ARGS) {
			auto size = CON_ARG_FLOAT(0);
			auto intensity = CON_ARG_FLOAT(1);
			auto outlined = CON_ARG_BOOL(2);
			auto rounding = CON_ARG_FLOAT(3);

			auto glass = std::make_shared<Shared::SceneHelpers::KillableByClick<Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Rounded<Scene::GrayscaledGlass>>>>>();
			glass->setSize(size);
			glass->setAnchor(0.5f);
			glass->setPivot(0.5f);
			glass->setOutlined(outlined);
			glass->setRounding(rounding);
			glass->setGrayscaleIntensity(intensity);
			sky::GetService<Scene::Scene>()->getRoot()->attach(glass);
		}));

		sky::AddCommand("spawn_shockwave", sky::CommandProcessor::Command(std::nullopt, {}, { { "duration", "1.0" } }, {}, [this](CON_ARGS) {
			auto duration = CON_ARG_FLOAT(0);

			auto shockwave = Shared::SceneHelpers::Shockwave::MakeAnimated(duration);
			shockwave->setSize(256.0f);
			shockwave->setAnchor(0.5f);
			shockwave->setPivot(0.5f);
			shockwave->setScale(4.0f);
			sky::GetService<Scene::Scene>()->getRoot()->attach(shockwave);
		}));

		sky::AddCommand("spawn_sprite_from_url", sky::CommandProcessor::Command(std::nullopt, {}, {}, { "url" }, [this](CON_ARGS) {
			auto url = CON_ARG_EXIST(0) ? CON_ARG(0) : std::string("https://raw.githubusercontent.com/okhmanyuk-ev/idle-phone-inc/master/art_src/ico.png");
			sky::Asset::Fetch(url, {
				.onSuccess = [url](const sky::Asset& asset) {
					auto image = Graphics::Image(asset);
					sky::GetService<sky::Cache>()->loadTexture(image, url);

					auto sprite = std::make_shared<Shared::SceneHelpers::KillableByClick<
					Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Sprite>>>>();
					sprite->setSize(256.0f);
					sprite->setAnchor(0.5f);
					sprite->setPivot(0.5f);
					sprite->setTexture(sky::GetTexture(url));
					sky::GetService<Scene::Scene>()->getRoot()->attach(sprite);
				}	
			});
		}));
	}

#if defined(BUILD_DEVELOPER)
	sky::ExecuteCommand("hud_show_fps 1");
	sky::ExecuteCommand("hud_show_drawcalls 1");
	sky::ExecuteCommand("hud_show_batches 1");
	sky::ExecuteCommand("hud_show_targets 1");
#else
	sky::GetService<sky::Console>()->setEnabled(false);
	sky::GetService<Shared::StatsSystem>()->setEnabled(false);
#endif

	std::vector<std::string> startup_commands;

	const auto& args = PLATFORM->getArguments();
	for (const auto& arg : args | std::views::drop(1))
	{
		sky::Log(sky::Console::Color::Gray, "processing argument: {}", arg);

		std::regex pattern(R"((\w+)=(\w+))"); // default web arguments 'http://localhost/?draft=true&lang=ru'
		std::smatch matches;

		if (std::regex_match(arg, matches, pattern))
		{
			auto key = matches[1].str();
			auto value = matches[2].str();
			mStartupKeyValues[key] = value;
			continue;
		}

		auto tokens = sky::GetService<sky::CommandProcessor>()->MakeTokensFromString(arg);

		if (tokens.empty())
			continue;

		auto& cmd = tokens.at(0);

		if (cmd.size() <= 1)
			continue;

		if (cmd.at(0) != '+')
			continue;

		cmd = cmd.substr(1);
		auto final_cmd = sky::GetService<sky::CommandProcessor>()->MakeStringFromTokens(tokens);
		startup_commands.push_back(final_cmd);
	}

	sky::Schedule(sky::ScheduleBehavior::Once, [startup_commands] {
		for (auto cmd : startup_commands)
			sky::GetService<sky::CommandProcessor>()->execute(cmd);
	});
}

Application::~Application()
{
	gCVarSceneTimestepEnabled.reset();
	gCVarSceneTimestepTimeCompletion.reset();
	sky::Locator<Shared::GestureDetector>::Reset();
	sky::Locator<Shared::TouchEmulator>::Reset();
	sky::Locator<Shared::ConsoleHelperCommands>::Reset();
	sky::Locator<Shared::PerformanceConsoleCommands>::Reset();
	sky::Locator<Shared::GraphicalConsoleCommands>::Reset();

	if (mFlags.count(Flag::Scene))
	{
		sky::Locator<Shared::SceneManager>::Reset();
		sky::Locator<Shared::SceneEditor>::Reset();
		sky::Locator<Scene::Scene>::Reset();
	}
	sky::Locator<Common::ConsoleCommands>::Reset();
	sky::Locator<Shared::ImScene>::Reset();
	sky::Locator<Shared::Stylebook>::Reset();
	sky::Locator<sky::ImguiSystem>::Reset();
	sky::Locator<sky::Cache>::Reset();
	if (mFlags.count(Flag::Audio))
	{
		sky::Locator<sky::Audio>::Reset();
	}
	sky::Locator<Shared::StatsSystem>::Reset();
	sky::Locator<sky::Localization>::Reset();
	if (mFlags.count(Flag::Network))
	{
#ifndef EMSCRIPTEN
		sky::Locator<Network::System>::Reset();
#endif
	}
	sky::Locator<Graphics::System>::Reset();
	sky::Locator<sky::Console>::Reset();
	sky::Locator<sky::Renderer>::Reset();
	sky::Locator<Platform::System>::Reset();
	sky::Locator<sky::Scheduler>::Reset();
	sky::Locator<sky::CommandProcessor>::Reset();
#ifndef EMSCRIPTEN
	sky::Locator<sky::ThreadPool>::Reset();
#endif
}

void Application::run()
{
	static auto frame = [&]{
		PLATFORM->process();

		if (PLATFORM->isFinished())
			return false;

		RENDERER->setRenderTarget(nullptr);
		RENDERER->clear();
		IMGUI_SYSTEM->begin();
		SCHEDULER->frame();
		if (mFlags.count(Flag::Scene))
		{
			sky::GetService<Scene::Scene>()->frame();
		}
		IMGUI_SYSTEM->end();
		RENDERER->present();

		return true;
	};

#ifdef EMSCRIPTEN
	emscripten_set_main_loop([] { frame(); }, 0, 1);
#else
	while (true)
	{
		if (!frame())
			break;
	}
#endif
}
