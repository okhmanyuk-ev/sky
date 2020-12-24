#include "scene_manager.h"

using namespace Shared;

SceneManager::SceneManager()
{
	setStretch(1.0f);

	mScreenHolder = std::make_shared<Scene::Node>();
	mScreenHolder->setStretch(1.0f);
	attach(mScreenHolder);

	mWindowHolder = std::make_shared<Scene::Node>();
	mWindowHolder->setStretch(1.0f);
	attach(mWindowHolder);
}

void SceneManager::switchScreen(std::shared_ptr<Screen> screen, Callback finishCallback)
{
	assert(!hasWindows());

	if (mInTransition)
		return;

	mInTransition = true;

	auto createLeaveAction = [this] {
		return Actions::Factory::MakeSequence(
			Actions::Factory::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaving;
				mCurrentScreen->onLeaveBegin();
			}),
			mCurrentScreen->createLeaveAction(),
				Actions::Factory::Execute([this] {
				mCurrentScreen->mState = Screen::State::Leaved;
				mCurrentScreen->onLeaveEnd();
				mScreenHolder->detach(mCurrentScreen);
				mCurrentScreen = nullptr;
			})
		);
	};

	auto createEnterAction = [this, screen] {
		return Actions::Factory::MakeSequence(
			Actions::Factory::Execute([this, screen] {
				mScreenHolder->attach(screen);
				mCurrentScreen = screen;
				mCurrentScreen->mState = Screen::State::Entering;
				mCurrentScreen->onEnterBegin();
			}),
			screen->createEnterAction(),
				Actions::Factory::Execute([this] {
				mInTransition = false;
				mCurrentScreen->mState = Screen::State::Entered;
				mCurrentScreen->onEnterEnd();
			})
		);
	};

	auto createFinalAction = [this, finishCallback] {
		return Actions::Factory::Execute([this, finishCallback] {
			mInTransition = false;
			if (finishCallback)
				finishCallback();
		});
	};

	if (screen == nullptr)
	{
		if (mCurrentScreen)
		{
			runAction(Actions::Factory::MakeSequence(
				createLeaveAction(),
				createFinalAction()
			));
		}
	}
	else
	{
		if (!mCurrentScreen)
		{
			runAction(Actions::Factory::MakeSequence(
				createEnterAction(),
				createFinalAction()
			));
		}
		else if (screen != mCurrentScreen)
		{
			runAction(Actions::Factory::MakeSequence(
				createLeaveAction(),
				createEnterAction(),
				createFinalAction()
			));
		}
	}
}

void SceneManager::pushWindow(std::shared_ptr<Window> window, Callback finishCallback)
{
	assert(window->getState() == Window::State::Closed);
	assert(!isWindowsBusy());

	if (mWindows.empty())
		mCurrentScreen->onWindowAppearing(); // only on first window in stack

	mWindows.push(window);
	mWindowHolder->attach(window);

	window->onOpenBegin();
	window->mState = Window::State::Opening;

	runAction(Actions::Factory::MakeSequence(
		window->createOpenAction(),
		Actions::Factory::Execute([window, finishCallback] {
			window->onOpenEnd();
			window->mState = Window::State::Opened;
			if (finishCallback)
			{
				finishCallback();
			}
		})
	));
}

void SceneManager::popWindow(size_t count, Callback finishCallback)
{
	if (count <= 0)
	{
		if (finishCallback)
		{
			finishCallback();
		}
		return;
	}

	assert(!mWindows.empty());
	assert(!isWindowsBusy());
	assert(count <= mWindows.size());

	auto window = mWindows.top();
	window->onCloseBegin();
	window->mState = Window::State::Closing;

	runAction(Actions::Factory::MakeSequence(
		window->createCloseAction(),
		Actions::Factory::Execute([this, window, count, finishCallback] {
			window->onCloseEnd();
			window->mState = Window::State::Closed;
			mWindowHolder->detach(window);
			mWindows.pop(); 
			
			if (mWindows.empty())
				mCurrentScreen->onWindowDisappearing(); // only on last window in stack

			popWindow(count - 1, finishCallback);
		})
	));
}

void SceneManager::popWindow(Callback finishCallback)
{
	popWindow(1, finishCallback);
}

bool SceneManager::isWindowsBusy() const
{
	if (mWindows.empty())
		return false;

	return mWindows.top()->getState() != Window::State::Opened;
}

bool SceneManager::isScreenBusy() const
{
	return mInTransition;
}