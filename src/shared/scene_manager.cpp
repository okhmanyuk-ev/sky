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
		return ActionHelpers::MakeSequence(
			ActionHelpers::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaving;
				mCurrentScreen->onLeaveBegin();
			}),
			mCurrentScreen->createLeaveAction(),
			ActionHelpers::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaved;
				mCurrentScreen->onLeaveEnd();
				detach(mCurrentScreen);
				mCurrentScreen = nullptr;
			})
		);
	};

	auto createEnterAction = [this, screen] {
		return ActionHelpers::MakeSequence(
			ActionHelpers::Execute([this, screen] {
				attach(screen);
				mCurrentScreen = screen;
				mCurrentScreen->mState = Screen::State::Entering;
				mCurrentScreen->onEnterBegin();
				mCurrentScreen->setSceneManager(weak_from_this());
			}),
			screen->createEnterAction(),
			ActionHelpers::Execute([this] {
				mInTransition = false;
				mCurrentScreen->mState = Screen::State::Entered;
				mCurrentScreen->onEnterEnd();
			})
		);
	};

	auto createFinalAction = [this, finishCallback] {
		return ActionHelpers::Execute([this, finishCallback] {
			mInTransition = false;
			if (finishCallback)
				finishCallback();
		});
	};

	if (screen == nullptr)
	{
		if (mCurrentScreen)
		{
			runAction(ActionHelpers::MakeSequence(
				createLeaveAction(),
				createFinalAction()
			));
		}
	}
	else
	{
		if (!mCurrentScreen)
		{
			runAction(ActionHelpers::MakeSequence(
				createEnterAction(),
				createFinalAction()
			));
		}
		else if (screen != mCurrentScreen)
		{
			runAction(ActionHelpers::MakeSequence(
				createLeaveAction(),
				createEnterAction(),
				createFinalAction()
			));
		}
	}
}

void SceneManager::pushWindow(std::shared_ptr<Window> window)
{
	assert(window->getState() == Window::State::Closed);
	assert(!isWindowsBusy());

	if (mWindows.empty())
		mCurrentScreen->onWindowAppearing(); // only on first window in stack

	mWindows.push(window);
	
	window->setSceneManager(weak_from_this());
	attach(window);

	window->onOpenBegin();
	window->mState = Window::State::Opening;

	runAction(ActionHelpers::MakeSequence(
		window->createOpenAction(),
		ActionHelpers::Execute([window] {
			window->onOpenEnd();
			window->mState = Window::State::Opened;
		})
	));
}

void SceneManager::popWindow(std::function<void()> finishCallback)
{
	assert(!mWindows.empty());
	assert(!isWindowsBusy());

	auto window = mWindows.top();

	window->onCloseBegin();
	window->mState = Window::State::Closing;

	runAction(ActionHelpers::MakeSequence(
		window->createCloseAction(),
		ActionHelpers::Execute([this, window, finishCallback] {
			window->onCloseEnd();
			window->mState = Window::State::Closed;
			detach(window);

			mWindows.pop(); 
			
			if (mWindows.empty())
				mCurrentScreen->onWindowDisappearing(); // only on last window in stack
			
			if (finishCallback)
				finishCallback();
		})
	));
}

bool SceneManager::isWindowsBusy() const
{
	if (mWindows.empty())
		return false;

	return mWindows.top()->getState() != Window::State::Opened;
}