#pragma once

#include <sky/locator.h>
#include <sky/console.h>
#include <sky/clock.h>
#include <functional>
#include <coroutine>
#include <optional>
#include <list>

namespace sky
{
	template<typename T = void>
	struct Task
	{
		template<typename Derived>
		struct PromiseBase
		{
			std::coroutine_handle<> prev;
			std::coroutine_handle<> last;

			PromiseBase* root{ this };

			struct FinalAwaiter
			{
				bool await_ready() const noexcept { return false; }
				void await_resume() noexcept {}

				template<typename promise_type>
				std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> current) noexcept
				{
					auto& promise = current.promise();
					return promise.prev ? promise.prev : std::noop_coroutine();
				}
			};

			Task<T> get_return_object() noexcept
			{
				auto handle = std::coroutine_handle<Derived>::from_promise(*(Derived*)this);
				auto task = Task<T>(handle);
				task.coroutine.promise().last = task.coroutine;
				return task;
			}

			auto initial_suspend() { return std::suspend_always{}; }
			auto final_suspend() noexcept(true) { return FinalAwaiter{}; }
			void unhandled_exception() { throw; }
		};

		template<typename U>
		struct Promise final : PromiseBase<Promise<U>>
		{
			std::optional<U> result;

			void return_value(U&& value)
			{
				result = std::forward<U>(value);
			}
		};

		template<>
		struct Promise<void> : PromiseBase<Promise<void>>
		{
			void return_void() noexcept {}
		};

		using promise_type = Promise<T>;
		using handle_type = std::coroutine_handle<promise_type>;

	private:
		handle_type coroutine;

	public:
		Task() = default;
		Task(handle_type handle) { coroutine = handle; }
		Task(Task const& other) = delete;
		Task& operator=(Task const& other) = delete;
	
		Task(Task&& other) noexcept : coroutine(std::exchange(other.coroutine, nullptr))
		{
		}
	
		Task& operator=(Task&& other)
		{
			if (this != &other)
			{
				if (coroutine)
					coroutine.destroy();
				coroutine = std::exchange(other.coroutine, nullptr);
			}
			return *this;
		}

		~Task()
		{
			if (coroutine)
				coroutine.destroy();
		}

		auto operator co_await() const& noexcept
		{
			struct Awaiter
			{
				handle_type current;

				bool await_ready() const noexcept { return false; }

				auto await_suspend(std::coroutine_handle<> prev) noexcept
				{
					auto& promise = current.promise();
					promise.prev = prev;
					promise.root = ((handle_type&)prev).promise().root;
					promise.root->last = current;
					return current;
				}

				auto await_resume()
				{
					if constexpr (!std::is_void<T>())
						return std::move(current.promise().result.value());
				}
			};

			return Awaiter{ coroutine };
		}

		T result() const
		{
			if constexpr (!std::is_void_v<T>)
				return coroutine.promise().result.value();
		}
	
		bool is_completed() const { return coroutine.done(); }

		void resume()
		{
			auto& last = coroutine.promise().last;
			assert(!last.done());
			last.resume();
		}
	};

	class Scheduler
	{
	public:
		static constexpr Locator<Scheduler>::Accessor Instance;

	public:
		void frame();
		void run(Task<>&& task);

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
		std::list<Task<>> mTasks;
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
}
