#include "rich_application.h"

#include <imgui.h>
#include <Shared/imgui_user.h>
#include <Common/easing.h>
#include <Shared/action_helpers.h>

using namespace Shared;

RichApplication::RichApplication(const std::string& appname) : GraphicalApplication(appname)
{
	ENGINE->addSystem<Audio::System>(std::make_shared<Audio::System>());

	STATS->setEnabled(false);
	CONSOLE_DEVICE->setEnabled(false);
	
	FRAME->addOne([this] {
		Common::Actions::Run(ActionHelpers::Delayed(1.0f,
			ActionHelpers::MakeSequence(
				ActionHelpers::Interpolate(1.0f, 0.0f, 0.5f, mLoadingFade),
				ActionHelpers::Execute([this] {
					mLoading = true;
					mStartLoadingTime = Clock::Now();
				})
			)
		));
	});

	FRAME->addInfinity([this] {
		if (mInitialized)
		{
			frame();
			return;
		}
		
		if (!mLoading)
			return;
	
		if (!mLoaded && !mLoadingTasks.empty())
		{
			mLoadingTasks[mLoadingPos].callback();
			mLoadingPos += 1;
		}
		
		if (mLoadingPos < mLoadingTasks.size())
			return;

		mLoaded = true;

		if (!mLoadingTasks.empty() && Clock::Now() - mStartLoadingTime <= Clock::FromSeconds(2.0f))
			return;

		mLoading = false;

		FRAME->addOne([this] {
			Common::Actions::Run(ActionHelpers::Delayed(0.25f,
				ActionHelpers::MakeSequence(
					ActionHelpers::Interpolate(0.0f, 1.0f, 0.5f, mLoadingFade),
					ActionHelpers::Execute([this] {
						initInternal();
						initialize();
						mInitialized = true;
					}),
					ActionHelpers::Interpolate(1.0f, 0.0f, 0.5f, mGameFade)
				)
			));
		});
	});
}

void RichApplication::postFrame()
{
	if (mGameFade > 0.0f)
	{
		GRAPHICS->begin();
		GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
		GRAPHICS->drawRectangle(glm::mat4(1.0f), { Graphics::Color::Black, mGameFade });
		GRAPHICS->pop();
		GRAPHICS->end();
	}

	if (mInitialized)
		return;

	if (!mLoadingTasks.empty())
	{
		auto stage = mLoadingTasks[mLoadingPos - (mLoaded ? 1 : 0)].name;
		auto progress = (float)(mLoadingPos) / (float)mLoadingTasks.size();
		loading(stage, progress);
	}
}

void RichApplication::postImguiPresent()
{
	if (mInitialized)
		return;

	if (mLoadingFade <= 0.0f)
		return;

	GRAPHICS->begin();
	GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
	GRAPHICS->drawRectangle(glm::mat4(1.0f), { Graphics::Color::Black, mLoadingFade });
	GRAPHICS->pop();
	GRAPHICS->end();
}

void RichApplication::initInternal()
{
	STATS->setEnabled(true);
	CONSOLE_DEVICE->setEnabled(true);
}

void RichApplication::loading(const std::string& stage, float progress)
{
	ImGui::Begin("Loading", nullptr, ImGui::User::ImGuiWindowFlags_Overlay & ~ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowPos(ImGui::User::BottomRightCorner());
	ImGui::SetWindowSize({ PLATFORM->getLogicalWidth() / 2.5f, -1.0f });
	ImGui::Text(stage.c_str());
	ImGui::ProgressBar(progress, { -1.0f, 3.0f });
	ImGui::End();
}

void RichApplication::addLoadingTasks(const std::vector<LoadingTask>& tasks)
{
	assert(!mLoading);
	assert(!mInitialized);
	for (const auto& task : tasks)
	{
		mLoadingTasks.push_back(task);
	}
}