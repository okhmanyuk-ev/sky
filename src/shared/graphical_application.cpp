#include "graphical_application.h"

#include <imgui.h>
#include <Shared/imgui_user.h>

using namespace Shared;

GraphicalApplication::GraphicalApplication(const std::string& appname)
{
	mTaskSystem = std::make_shared<Common::TaskSystem>();
	ENGINE->setTask(&(*mTaskSystem));

	mFrame = std::make_shared<Common::FrameSystem>();
	ENGINE->setFrame(&(*mFrame));

	mEvent = std::make_shared<Common::EventSystem>();
	ENGINE->setEvent(&(*mEvent));

	mPlatform = Platform::System::create(appname);
	ENGINE->setPlatform(&(*mPlatform));

	mRenderer = std::make_shared<RendererDebug<Renderer::SystemCrossplatform>>();
	ENGINE->setRenderer(&(*mRenderer));

	mConsoleDevice = std::make_shared<Shared::ConsoleDevice>();
	ENGINE->setConsoleDevice(&(*mConsoleDevice));

	mConsole = std::make_shared<Console::System>();
	ENGINE->setConsole(&(*mConsole));

	mGraphics = std::make_shared<Graphics::System>();
	ENGINE->setGraphics(&(*mGraphics));

	mImguiSystem = std::make_shared<Shared::ImguiSystem>();
	
	mNetwork = std::make_shared<Network::System>();
	ENGINE->setNetwork(&(*mNetwork));

	mConsoleCommands = std::make_shared<Common::ConsoleCommands>();
	mGraphicalConsoleCommands = std::make_shared<Shared::GraphicalConsoleCommands>();

	mPerformanceConsoleCommands = std::make_shared<Shared::PerformanceConsoleCommands>();
	mConsoleHelperCommands = std::make_shared<Shared::ConsoleHelperCommands>();

	mLocalization = std::make_shared<Shared::LocalizationSystem>();
	ENGINE->setLocalization(&(*mLocalization));

	mStats = std::make_shared<Shared::StatsSystem>();
	ENGINE->setStats(&(*mStats));

	mCache = std::make_shared<Shared::CacheSystem>();
	ENGINE->setCache(&(*mCache));

	mTouchEmulator = std::make_shared<Shared::TouchEmulator>();
	mGestureDetector = std::make_shared<Shared::GestureDetector>();

	//

	ImGui::User::SetupStyleFromColor(1.0f, 1.0f, 0.75f);
	
	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;

	mConsoleCommands->setQuitCallback([this] {
		mPlatform->quit();
	});
}

void GraphicalApplication::run()
{
	while (!mPlatform->isFinished())
	{
		mPlatform->process();
		mRenderer->setRenderTarget(nullptr);
		mRenderer->clear();
		preFrame();
		mFrame->frame();
		postFrame();
		mImguiSystem->present();
		postImguiPresent();
		mRenderer->present();
	}
}