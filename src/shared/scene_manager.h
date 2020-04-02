#pragma once

#include <shared/all.h>
#include "scene_manager.h"

namespace Shared
{
	class SceneManager : public Scene::Actionable<Scene::Node>, public std::enable_shared_from_this<SceneManager>
	{
	public:
		class Screen;
		class Window;

	public:
		SceneManager();

	public:
		void switchScreen(std::shared_ptr<Screen> screen, std::function<void()> finishCallback = nullptr);
		
		void pushWindow(std::shared_ptr<Window> window);
		void popWindow(std::function<void()> finishCallback = nullptr);
		
		size_t getOpenedWindowsCount() const { return mWindows.size(); }
		bool hasOpenedWindows() const { return getOpenedWindowsCount() > 0; }

		bool isWindowsBusy() const;

		auto getScreenHolder() { return mScreenHolder; }
		auto getWindowHolder() { return mWindowHolder; }

	private:
		std::shared_ptr<Scene::Node> mScreenHolder = nullptr;
		std::shared_ptr<Scene::Node> mWindowHolder = nullptr;
		std::shared_ptr<Screen> mCurrentScreen = nullptr;
		std::stack<std::shared_ptr<Window>> mWindows;
		bool mInTransition = false;
	};

	class SceneManager::Screen : public Scene::Node
	{
		friend SceneManager;
	public:
		enum class State
		{
			Leaving,
			Leaved,
			Entering,
			Entered,
		};

	public:
		auto getState() const { return mState; }

	private:
		State mState = State::Leaved;

	protected:
		virtual void onEnterBegin() { }
		virtual void onEnterEnd() { }
		virtual void onLeaveBegin() { }
		virtual void onLeaveEnd() { }

		virtual void onWindowAppearing() { }
		virtual void onWindowDisappearing() { }

	public:
		virtual std::unique_ptr<Common::Actions::Action> createEnterAction() = 0;
		virtual std::unique_ptr<Common::Actions::Action> createLeaveAction() = 0;

	public:
		auto getSceneManager() const { return mSceneManager.lock(); }

	private:
		void setSceneManager(std::weak_ptr<SceneManager> value) { mSceneManager = value; }

	private:
		std::weak_ptr<SceneManager> mSceneManager;
	};

	class SceneManager::Window : public Scene::Node
	{
		friend SceneManager;

	public:
		enum class State
		{
			Opening,
			Opened,
			Closing,
			Closed,
		};

	public:
		auto getState() const { return mState; }

	private:
		State mState = State::Closed;

	protected:
		virtual void onOpenBegin() { }
		virtual void onOpenEnd() { }
		virtual void onCloseBegin() { }
		virtual void onCloseEnd() { }

	public:
		virtual std::unique_ptr<Common::Actions::Action> createOpenAction() = 0;
		virtual std::unique_ptr<Common::Actions::Action> createCloseAction() = 0;

	public:
		auto getSceneManager() const { return mSceneManager.lock(); }

	private:
		void setSceneManager(std::weak_ptr<SceneManager> value) { mSceneManager = value; }

	private:
		std::weak_ptr<SceneManager> mSceneManager;
	};
}
