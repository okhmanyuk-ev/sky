#pragma once

#include <Core/engine.h>
#include <list>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

namespace Common
{
	class TaskSystem
	{
	public:
		TaskSystem(int threadsCount);
		TaskSystem();
		~TaskSystem();

	public:
		template<class F, class... Args> auto addTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

	public:
		auto getTasksCount() const { return mTasks.size() + mBusyThreads; }
		auto getThreadsCount() const { return mThreads.size(); }

	private:
		std::list<std::function<void()>> mTasks;
		std::list<std::thread> mThreads;
		std::mutex mMutex;
		std::condition_variable mCondition;
		bool mFinished = false;
		std::atomic<int> mBusyThreads = 0;
	};

	template<class F, class... Args> auto TaskSystem::addTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...));
		
		auto res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mTasks.emplace_back([task] { (*task)(); });
		}

		mCondition.notify_one();
		return res;
	}
}