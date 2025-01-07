#pragma once

#include <scene/all.h>

#define SCENE_MANAGER sky::Singleton<Shared::SceneManager>::GetInstance()

namespace Shared
{
	class SceneManager : public Scene::Node
	{
	public:
		class Screen;
		class Window;
	
	public:
		using Callback = std::function<void()>;

	public:
		SceneManager();

	public:
		void switchScreen(std::shared_ptr<Screen> screen, Callback finishCallback = nullptr);
		void switchScreenBack(Callback finishCallback = nullptr);

		void pushWindow(std::shared_ptr<Window> window, Callback finishCallback = nullptr);
		void popWindow(size_t count = 1, Callback finishCallback = nullptr);
		void popWindow(Callback finishCallback);
		
		size_t getWindowsCount() const { return mWindows.size(); }
		bool hasWindows() const { return getWindowsCount() > 0; }

		bool isWindowsBusy() const;
		bool isScreenBusy() const;

		auto getScreenHolder() { return mScreenHolder; }
		auto getWindowHolder() { return mWindowHolder; }

	private:
		std::shared_ptr<Scene::Node> mScreenHolder = nullptr;
		std::shared_ptr<Scene::Node> mWindowHolder = nullptr;
		std::shared_ptr<Screen> mCurrentScreen = nullptr;
		std::stack<std::shared_ptr<Window>> mWindows;
		bool mInTransition = false;
		std::weak_ptr<Screen> mPrevScreen;
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

		virtual void onWindowAppearingBegin() { }
		virtual void onWindowAppearingEnd() { }
		virtual void onWindowDisappearingBegin() { }
		virtual void onWindowDisappearingEnd() { }

	public:
		virtual std::unique_ptr<Actions::Action> createEnterAction() = 0;
		virtual std::unique_ptr<Actions::Action> createLeaveAction() = 0;
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
		virtual std::unique_ptr<Actions::Action> createOpenAction() = 0;
		virtual std::unique_ptr<Actions::Action> createCloseAction() = 0;
	};
}
