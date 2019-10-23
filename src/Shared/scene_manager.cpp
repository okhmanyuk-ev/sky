#include "scene_manager.h"

using namespace Shared;

SceneManager::SceneManager()
{
	setStretch({ 1.0f, 1.0f });
}

void SceneManager::switchScreen(std::shared_ptr<Screen> screen, std::function<void()> finishCallback)
{
	assert(!hasOpenedWindows());

	if (mInTransition)
		return;

	mInTransition = true;

	auto createLeaveAction = [this] {
		return Shared::CommonActions::MakeSequence(
			Shared::CommonActions::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaving;
				mCurrentScreen->onLeaveBegin();
			}),
			mCurrentScreen->createLeaveAction(),
			Shared::CommonActions::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaved;
				mCurrentScreen->onLeaveEnd();
				detach(mCurrentScreen);
				mCurrentScreen = nullptr;
			})
		);
	};

	auto createEnterAction = [this, screen] {
		return Shared::CommonActions::MakeSequence(
			Shared::CommonActions::Execute([this, screen] {
				attach(screen);
				mCurrentScreen = screen;
				mCurrentScreen->mState = Screen::State::Entering;
				mCurrentScreen->onEnterBegin();
				mCurrentScreen->setSceneManager(weak_from_this());
			}),
			screen->createEnterAction(),
			Shared::CommonActions::Execute([this] {
				mInTransition = false;
				mCurrentScreen->mState = Screen::State::Entered;
				mCurrentScreen->onEnterEnd();
			})
		);
	};

	auto createFinalAction = [this, finishCallback] {
		return Shared::CommonActions::Execute([this, finishCallback] {
			mInTransition = false;
			if (finishCallback)
				finishCallback();
		});
	};

	if (screen == nullptr)
	{
		if (mCurrentScreen)
		{
			runAction(Shared::CommonActions::MakeSequence(
				createLeaveAction(),
				createFinalAction()
			));
		}
	}
	else
	{
		if (!mCurrentScreen)
		{
			runAction(Shared::CommonActions::MakeSequence(
				createEnterAction(),
				createFinalAction()
			));
		}
		else if (screen != mCurrentScreen)
		{
			runAction(Shared::CommonActions::MakeSequence(
				createLeaveAction(),
				createEnterAction(),
				createFinalAction()
			));
		}
	}
}

void SceneManager::pushWindow(std::shared_ptr<Window> window)
{
	assert(!mCurrentWindow);

	mCurrentScreen->onWindowAppearing(); // TODO: call this on first window open (stack must be included)
	mCurrentWindow = window;
	mCurrentWindow->setSceneManager(weak_from_this());
	attach(mCurrentWindow);

	mCurrentWindow->onOpenBegin();

	runAction(Shared::CommonActions::MakeSequence(
		mCurrentWindow->createOpenAction(),
		Shared::CommonActions::Execute([this] {
			mCurrentWindow->onOpenEnd();
		})
	));
}

void SceneManager::popWindow(std::function<void()> finishCallback)
{
	assert(mCurrentWindow);

	mCurrentWindow->onCloseBegin();

	runAction(Shared::CommonActions::MakeSequence(
		mCurrentWindow->createCloseAction(),
		Shared::CommonActions::Execute([this, finishCallback] {
			mCurrentWindow->onCloseEnd();
			detach(mCurrentWindow);
			mCurrentWindow = nullptr; 
			mCurrentScreen->onWindowDisappearing(); // TODO: only on last window close (when stack will be included)
			if (finishCallback)
				finishCallback();
		})
	));
}