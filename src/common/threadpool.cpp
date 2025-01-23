#include "threadpool.h"
#include <algorithm>

using namespace Common;

ThreadPool::ThreadPool(int threadsCount)
{
	if (threadsCount <= 0)
		threadsCount = 1;

	for (int i = 0; i < threadsCount; i++)
	{
		mThreads.emplace_back([this] {
			while (true)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(mMutex);
					mCondition.wait(lock, [this] {
						return mFinished || !mTasks.empty();
					});

					if (mFinished && mTasks.empty())
						return;

					task = std::move(mTasks.front());
					mTasks.pop_front();
				}
				mBusyThreads++;
				task();
				mBusyThreads--;
			}
		});
	}
}

ThreadPool::ThreadPool() : ThreadPool(std::max<int>(1, std::thread::hardware_concurrency() - 1))
{
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(mMutex);
		mFinished = true;
	}

	mCondition.notify_all();

	for (auto& thread : mThreads)
		thread.join();
}