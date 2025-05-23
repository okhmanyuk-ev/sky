#pragma once

#include <list>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

#define THREADPOOL sky::Locator<sky::ThreadPool>::Get()

namespace sky
{
	class ThreadPool
	{
	public:
		ThreadPool(int threadsCount);
		ThreadPool();
		~ThreadPool();

	public:
		template<class F, class... Args> auto addTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

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

	template<class F, class... Args> auto ThreadPool::addTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mTasks.emplace_back([task] {
				(*task)();
			});
		}
		mCondition.notify_one();
		return task->get_future();
	}
}