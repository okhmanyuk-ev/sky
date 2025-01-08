#include "application.h"
#include <imgui.h>
#include <shared/imgui_user.h>
#include <shared/imscene.h>
#include <audio/system.h>
#include <shared/scene_manager.h>
#include <shared/scene_helpers.h>
#include <regex>
#include <sky/locator.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif

using namespace Shared;

Application::Application(const std::string& appname, const Flags& flags) : mFlags(flags)
{
	std::srand((unsigned int)std::time(nullptr));

	sky::Locator<Common::Event::System>::Init(std::make_shared<Common::Event::System>());
#ifndef EMSCRIPTEN
	sky::Locator<Common::TaskSystem>::Init(std::make_shared<Common::TaskSystem>());
#endif
	sky::Locator<Common::FrameSystem>::Init(std::make_shared<Common::FrameSystem>());
	sky::Locator<Common::ProfilerSystem>::Init(std::make_shared<Common::ProfilerSystem>());
	sky::Locator<Platform::System>::Init(Platform::System::create(appname));
	sky::Locator<Renderer::System>::Init(std::make_shared<Renderer::System>());
	sky::Locator<Console::Device>::Init(std::make_shared<Shared::ConsoleDevice>());
	sky::Locator<Console::System>::Init(std::make_shared<Console::System>());
	sky::Locator<Graphics::System>::Init(std::make_shared<Graphics::System>());
	if (flags.count(Flag::Network))
	{
		sky::Locator<Network::System>::Init(std::make_shared<Network::System>());
	}
	sky::Locator<Shared::LocalizationSystem>::Init(std::make_shared<Shared::LocalizationSystem>());
	sky::Locator<Shared::StatsSystem>::Init(std::make_shared<Shared::StatsSystem>());
	sky::Locator<Shared::CacheSystem>::Init(std::make_shared<Shared::CacheSystem>());
	sky::Locator<Shared::ImguiSystem>::Init(std::make_shared<Shared::ImguiSystem>());
	sky::Locator<Shared::Stylebook>::Init(std::make_shared<Shared::Stylebook>());
	sky::Locator<Shared::ImScene>::Init(std::make_shared<Shared::ImScene>());
	if (flags.count(Flag::Audio))
	{
		sky::Locator<Audio::System>::Init(std::make_shared<Audio::System>());
	}

	Scene::RichLabel::DefaultIconTextureCallback = [](const auto& path) { return TEXTURE(path); };

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

	IMGUI_SYSTEM->setSamplerNearest(false);

	if (flags.count(Flag::Scene))
	{
		mScene = std::make_shared<Scene::Scene>();
		mScene->setInteractTestCallback([](const auto& pos) {
			return !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup) 
				&& !ImGui::IsAnyItemActive();
		});
		mSceneEditor = std::make_shared<SceneEditor>(*mScene);

		auto scene_manager = std::make_shared<Shared::SceneManager>();
		sky::Locator<Shared::SceneManager>::Init(scene_manager);
		mScene->getRoot()->attach(scene_manager);

		auto getter = [this] {
			auto fps = 1.0f / sky::ToSeconds(mScene->getTimestepFixer().getTimestep());
			return std::vector<std::string>({ std::to_string(fps) });
		};

		auto setter = [this](CON_ARGS) {
			auto sec = std::stof(CON_ARG(0));
			mScene->getTimestepFixer().setTimestep(sky::FromSeconds(1.0f / sec));
		};

		CONSOLE->registerCVar("scene_timestep_fps", { "float" }, getter, setter);

		CONSOLE->registerCVar("scene_timestep_enabled", { "bool" }, 
			CVAR_GETTER_BOOL_FUNC(mScene->getTimestepFixer().isEnabled), 
			CVAR_SETTER_BOOL_FUNC(mScene->getTimestepFixer().setEnabled));

		CONSOLE->registerCVar("scene_timestep_force_time_completion", { "bool" },
			CVAR_GETTER_BOOL_FUNC(mScene->getTimestepFixer().getForceTimeCompletion),
			CVAR_SETTER_BOOL_FUNC(mScene->getTimestepFixer().setForceTimeCompletion));

		CONSOLE->registerCommand("spawn_blur_glass", std::nullopt, {}, { "intensity", "passes", "outlined", "rounding" }, [this](CON_ARGS) {
			float intensity = 0.5f;

			if (CON_ARG_EXIST(0))
				intensity = CON_ARG_FLOAT(0);

			int passes = 1;

			if (CON_ARG_EXIST(1))
				passes = CON_ARG_INT(1);

			bool outlined = true;

			if (CON_ARG_EXIST(2))
				outlined = CON_ARG_BOOL(2);

			float rounding = 0.0f;

			if (CON_ARG_EXIST(3))
				rounding = CON_ARG_FLOAT(3);

			auto blur = std::make_shared<Shared::SceneHelpers::KillableByClick<Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Rounded<Scene::BlurredGlass>>>>>();
			blur->setSize(192.0f);
			blur->setAnchor(0.5f);
			blur->setPivot(0.5f);
			blur->setOutlined(outlined);
			blur->setBlurIntensity(intensity);
			blur->setBlurPasses(passes);
			blur->setRounding(rounding);
			getScene()->getRoot()->attach(blur);
		});

		CONSOLE->registerCommand("spawn_gray_glass", std::nullopt, {}, { "intensity", "outlined", "rounding" }, [this](CON_ARGS) {
			float intensity = 1.0f;

			if (CON_ARG_EXIST(0))
				intensity = CON_ARG_FLOAT(0);

			bool outlined = true;

			if (CON_ARG_EXIST(1))
				outlined = CON_ARG_BOOL(1);

			float rounding = 0.0f;

			if (CON_ARG_EXIST(2))
				rounding = CON_ARG_FLOAT(2);

			auto gray = std::make_shared<Shared::SceneHelpers::KillableByClick<Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Rounded<Scene::GrayscaledGlass>>>>>();
			gray->setSize(192.0f);
			gray->setAnchor(0.5f);
			gray->setPivot(0.5f);
			gray->setOutlined(outlined);
			gray->setRounding(rounding);
			gray->setGrayscaleIntensity(intensity);
			getScene()->getRoot()->attach(gray);
		});

		CONSOLE->registerCommand("spawn_shockwave", std::nullopt, {}, { "duration" }, [this](CON_ARGS) {
			float duration = 1.0f;

			if (CON_ARG_EXIST(0))
				duration = CON_ARG_FLOAT(0);

			auto shockwave = Shared::SceneHelpers::Shockwave::MakeAnimated(duration);
			shockwave->setSize(256.0f);
			shockwave->setAnchor(0.5f);
			shockwave->setPivot(0.5f);
			shockwave->setScale(4.0f);
			getScene()->getRoot()->attach(shockwave);
		});

		CONSOLE->registerCommand("spawn_sprite_from_url", std::nullopt, {}, { "url" }, [this](CON_ARGS) {
#ifndef EMSCRIPTEN
			sky::Log("this is for emscripten");
#else
			static auto spawn_sprite = [this](const std::string& url){
				auto sprite = std::make_shared<Shared::SceneHelpers::KillableByClick<Shared::SceneHelpers::MovableByHand<Shared::SceneHelpers::Outlined<Scene::Sprite>>>>();
				sprite->setSize(256.0f);
				sprite->setAnchor(0.5f);
				sprite->setPivot(0.5f);
				sprite->setTexture(TEXTURE(url));
				getScene()->getRoot()->attach(sprite);
			};

			auto download_succeeded = [](emscripten_fetch_t* fetch) {
				sky::Log("download succeeded");

				auto data = fetch->data;
				auto size = fetch->numBytes;
				auto url = fetch->url;
				auto image = Graphics::Image((void*)data, (size_t)size);

				CACHE->loadTexture(image, url);
				spawn_sprite(url);
				emscripten_fetch_close(fetch);
			};

			auto download_failed = [](emscripten_fetch_t* fetch) {
				sky::Log("download failed");
				emscripten_fetch_close(fetch);
			};

			auto download_progress = [](emscripten_fetch_t* fetch) {
				sky::Log("download progress {} of {}", fetch->dataOffset, fetch->totalBytes);
			};

			auto url = CON_ARG_EXIST(0) ? CON_ARG(0) : std::string("https://raw.githubusercontent.com/okhmanyuk-ev/idle-phone-inc/master/art_src/ico.png");

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
			attr.onsuccess = download_succeeded;
			attr.onerror = download_failed;
			attr.onprogress = download_progress;
			emscripten_fetch(&attr, url.c_str());

			sky::Log("fetching {}", url);
#endif
		});
	}

#if defined(BUILD_DEVELOPER)
	CONSOLE->execute("hud_show_fps 1");
	CONSOLE->execute("hud_show_drawcalls 1");
	CONSOLE->execute("hud_show_batches 1");
	CONSOLE->execute("hud_show_targets 1");
#else
	CONSOLE_DEVICE->setEnabled(false);
	STATS->setEnabled(false);
#endif

	std::vector<std::string> startup_commands;

	const auto& args = PLATFORM->getArguments();
	for (const auto& arg : args | std::views::drop(1))
	{
		sky::Log(Console::Color::Gray, "processing argument: {}", arg);

		std::regex pattern(R"((\w+)=(\w+))"); // default web arguments 'http://localhost/?draft=true&lang=ru'
		std::smatch matches;

		if (std::regex_match(arg, matches, pattern))
		{
			auto key = matches[1].str();
			auto value = matches[2].str();
			mStartupKeyValues[key] = value;
			continue;
		}

		auto tokens = CONSOLE->MakeTokensFromString(arg);

		if (tokens.empty())
			continue;

		auto& cmd = tokens.at(0);

		if (cmd.size() <= 1)
			continue;

		if (cmd.at(0) != '+')
			continue;

		cmd = cmd.substr(1);
		auto final_cmd = CONSOLE->MakeStringFromTokens(tokens);
		startup_commands.push_back(final_cmd);
	}

	FRAME->addOne([startup_commands] {
		for (auto cmd : startup_commands)
			CONSOLE->execute(cmd);
	});
}

Application::~Application()
{
	if (mFlags.count(Flag::Scene))
	{
		sky::Locator<Shared::SceneManager>::Reset();
		mSceneEditor = nullptr;
		mScene = nullptr;
	}
	if (mFlags.count(Flag::Audio))
	{
		sky::Locator<Audio::System>::Reset();
	}
	sky::Locator<Shared::ImScene>::Reset();
	sky::Locator<Shared::Stylebook>::Reset();
	sky::Locator<Shared::ImguiSystem>::Reset();
	sky::Locator<Shared::CacheSystem>::Reset();
	sky::Locator<Shared::StatsSystem>::Reset();
	sky::Locator<Shared::LocalizationSystem>::Reset();
	if (mFlags.count(Flag::Network))
	{
		sky::Locator<Network::System>::Reset();
	}
	sky::Locator<Graphics::System>::Reset();
	sky::Locator<Console::System>::Reset();
	sky::Locator<Console::Device>::Reset();
	sky::Locator<Renderer::System>::Reset();
	sky::Locator<Platform::System>::Reset();
	sky::Locator<Common::ProfilerSystem>::Reset();
	sky::Locator<Common::FrameSystem>::Reset();
#ifndef EMSCRIPTEN
	sky::Locator<Common::TaskSystem>::Reset();
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
		if (mFlags.count(Flag::Scene))
		{
			mScene->frame();
		}
		FRAME->frame();
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
