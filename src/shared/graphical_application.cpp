#include "graphical_application.h"

#include <imgui.h>
#include <Shared/imgui_user.h>

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

	mImguiSystem = std::make_shared<Shared::ImguiSystem>();
	mConsoleCommands = std::make_shared<Common::ConsoleCommands>();
	mGraphicalConsoleCommands = std::make_shared<Shared::GraphicalConsoleCommands>();
	mPerformanceConsoleCommands = std::make_shared<Shared::PerformanceConsoleCommands>();
	mConsoleHelperCommands = std::make_shared<Shared::ConsoleHelperCommands>();
	mTouchEmulator = std::make_shared<Shared::TouchEmulator>();
	mGestureDetector = std::make_shared<Shared::GestureDetector>();

	//

	ImGui::User::SetupStyleFromColor(1.0f, 1.0f, 0.75f);
	
	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;

	mConsoleCommands->setQuitCallback([this] {
		PLATFORM->quit();
	});
}

GraphicalApplication::~GraphicalApplication()
{
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
		preFrame();
		FRAME->frame();
		postFrame();
		mImguiSystem->present();
		postImguiPresent();
		RENDERER->present();
	}
}