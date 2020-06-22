#include "graphical_application.h"

#include <imgui.h>
#include <shared/imgui_user.h>

using namespace Shared;

GraphicalApplication::GraphicalApplication(const std::string& appname)
{
	ENGINE->addSystem<Common::EventSystem>(std::make_shared<Common::EventSystem>());
	ENGINE->addSystem<Common::TaskSystem>(std::make_shared<Common::TaskSystem>());
	ENGINE->addSystem<Common::FrameSystem>(std::make_shared<Common::FrameSystem>());
	ENGINE->addSystem<Common::ProfilerSystem>(std::make_shared<Common::ProfilerSystem>());
	ENGINE->addSystem<Platform::System>(Platform::System::create(appname));
	ENGINE->addSystem<Renderer::System>(std::make_shared<RendererDebug<Renderer::SystemCrossplatform>>());
	ENGINE->addSystem<Console::Device>(std::make_shared<Shared::ConsoleDevice>());
	ENGINE->addSystem<Console::System>(std::make_shared<Console::System>());
	ENGINE->addSystem<Graphics::System>(std::make_shared<Graphics::System>());
	ENGINE->addSystem<Network::System>(std::make_shared<Network::System>());
	ENGINE->addSystem<Shared::LocalizationSystem>(std::make_shared<Shared::LocalizationSystem>());
	ENGINE->addSystem<Shared::StatsSystem>(std::make_shared<Shared::StatsSystem>());
	ENGINE->addSystem<Shared::CacheSystem>(std::make_shared<Shared::CacheSystem>());
	ENGINE->addSystem<Shared::ImguiSystem>(std::make_shared<Shared::ImguiSystem>());
	ENGINE->addSystem<Shared::Stylebook>(std::make_shared<Shared::Stylebook>());
	
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
}

GraphicalApplication::~GraphicalApplication()
{
	ENGINE->removeSystem<Shared::Stylebook>();
	ENGINE->removeSystem<Shared::ImguiSystem>();
	ENGINE->removeSystem<Shared::CacheSystem>();
	ENGINE->removeSystem<Shared::StatsSystem>();
	ENGINE->removeSystem<Shared::LocalizationSystem>();
	ENGINE->removeSystem<Network::System>();
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

void GraphicalApplication::run()
{
	while (!PLATFORM->isFinished())
	{
		PLATFORM->process();
		RENDERER->setRenderTarget(nullptr);
		RENDERER->clear();
		IMGUI_SYSTEM->begin();
		FRAME->frame();
		IMGUI_SYSTEM->end();
		RENDERER->present();
	}
}