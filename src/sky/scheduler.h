#pragma once

#include <sky/locator.h>
#include <sky/console.h>
#include <sky/clock.h>
#include <functional>
#include <list>

namespace sky
{
	class Scheduler
	{
	public:
		static constexpr Locator<Scheduler>::Accessor Instance;

		enum class Status
		{
			Finished,
			Continue
		};

		class Task;

	public:
		void frame();
		void run(Task task);

	public:
		int getFramerateLimit() const { return mFramerateLimit; }
		void setFramerateLimit(int value) { mFramerateLimit = value; }

		bool isSleepAllowed() const { return mSleepAllowed; }
		void setSleepAllowed(bool value) { mSleepAllowed = value; }

		auto getTimeDelta() const { return mTimeDelta; }

		float getTimeScale() const { return mTimeScale; }
		void setTimeScale(float value) { mTimeScale = value; }

		auto getFramerate() const { return 1.0f / sky::ToSeconds(mTimeDelta) * mTimeScale; } // frame count per second
		auto getTasksCount() const { return mTasks.size(); }

		auto getUptime() const { return mUptime; }
		auto getFrameCount() { return mFrameCount; }

		auto getTimeDeltaLimit() const { return mTimeDeltaLimit; }
		void setTimeDeltaLimit(std::optional<sky::Duration> value) { mTimeDeltaLimit = value; }

		auto isChoked() const { return mChoked; }

	private:
		std::list<Task> mTasks;
		sky::CVar<int> mFramerateLimit = sky::CVar<int>("sys_framerate", 0, "limit of fps");
		sky::CVar<bool> mSleepAllowed = sky::CVar<bool>("sys_sleep", true, "cpu saving between frames");
		sky::CVar<float> mTimeScale = sky::CVar<float>("sys_timescale", 1.0f, "time delta multiplier");
		sky::TimePoint mLastTime = sky::Now();
		sky::Duration mTimeDelta = sky::Duration::zero();
		sky::Duration mUptime = sky::Duration::zero();
		std::optional<sky::Duration> mTimeDeltaLimit; // this can save from animation breaks
		uint64_t mFrameCount = 0;
		bool mChoked = false;
	};

	class Scheduler::Task
	{
	public:
		Task(std::initializer_list<Task> tasks)
		{
			mFunc = [tasks = std::list<Task>(std::move(tasks))]() mutable {
				if (tasks.empty())
					return Status::Finished;

				if (tasks.front()() == Status::Continue)
					return Status::Continue;

				tasks.pop_front();
				return tasks.empty() ? Status::Finished : Status::Continue;
			};
		}

		template <typename Func>
			requires std::invocable<Func> &&
			(std::same_as<std::invoke_result_t<Func>, Status> || std::same_as<std::invoke_result_t<Func>, void>)
		Task(Func&& func)
		{			
			mFunc = [func = std::forward<Func>(func)]() mutable {
				if constexpr (std::same_as<std::invoke_result_t<Func>, Status>)
				{
					return func();
				}
				else
				{
					func();
					return Status::Finished;
				}
			};
		}

		template <typename Func>
			requires std::invocable<Func> && (std::convertible_to<std::invoke_result_t<Func>, std::tuple<Status, std::optional<Task>>> ||
				std::convertible_to<std::invoke_result_t<Func>, std::optional<Task>>)
		Task(Func&& func)
		{
			mFunc = [func = std::forward<Func>(func), need_invoke_func = true, status = std::optional<Status>{ std::nullopt }, task = std::optional<Task>{ std::nullopt }]() mutable {
				if (need_invoke_func)
				{
					if constexpr (std::convertible_to<std::invoke_result_t<Func>, std::tuple<Status, std::optional<Task>>>)
					{
						std::tie(status, task) = func();
					}
					else
					{
						status = Status::Finished;
						task = func();
					}
					
					need_invoke_func = false;
				
					if (task.has_value())
						return Status::Continue;
				}
				
				if (task.has_value())
				{
					if (task.value()() == Status::Continue)
						return Status::Continue;
				}
				
				need_invoke_func = true;
				return status.value();
			};
		}

		auto operator()()
		{
			return mFunc();
		}

	private:
		std::function<Status()> mFunc;
	};
}
