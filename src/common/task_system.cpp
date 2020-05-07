#include "task_system.h"
#include <algorithm>

using namespace Common;

TaskSystem::TaskSystem(int threadsCount)
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

TaskSystem::TaskSystem() : TaskSystem(std::max<int>(1, std::thread::hardware_concurrency() - 1))
{
	//
}

TaskSystem::~TaskSystem()
{
	{
		std::unique_lock<std::mutex> lock(mMutex);
		mFinished = true;
	}
	
	mCondition.notify_all();

	for (auto& thread : mThreads)
		thread.join();
}