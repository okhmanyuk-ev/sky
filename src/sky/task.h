#pragma once

#include <coroutine>
#include <concepts>
#include <functional>
#include <optional>
#include <utility>
#include <exception>
#include <cassert>
#include <type_traits>

namespace sky
{
	template<typename T>
	class Task;

	template<typename T>
	struct is_task : std::false_type {};

	template<typename T>
	struct is_task<Task<T>> : std::true_type {};

	struct TaskPromiseBase
	{
		std::coroutine_handle<> prev;
		std::coroutine_handle<> last;
		TaskPromiseBase* root{ this };
		std::exception_ptr eptr;

		struct FinalAwaiter
		{
			bool await_ready() const noexcept { return false; }
			void await_resume() noexcept {}

			template<typename T>
			std::coroutine_handle<> await_suspend(std::coroutine_handle<T> current) noexcept
			{
				auto& promise = current.promise();

				if (!promise.prev)
					return std::noop_coroutine();

				promise.root->last = promise.prev;
				return promise.prev;
			}
		};

		auto initial_suspend() { return std::suspend_always{}; }
		auto final_suspend() noexcept(true) { return FinalAwaiter{}; }

		void unhandled_exception() { eptr = std::current_exception(); }

		template<typename T>
			requires (!is_task<std::remove_cvref_t<T>>::value)
		auto await_transform(T&& a) noexcept
		{
			return std::forward<T>(a);
		}

		template<typename T>
		auto await_transform(Task<T>&& task) noexcept;
	};

	template<typename T = void>
	class [[nodiscard]] Task
	{
	public:
		template<typename U>
		struct Promise final : TaskPromiseBase
		{
			std::optional<U> result;

			void return_value(U value) { result = std::move(value); }

			Task<U> get_return_object() noexcept
			{
				auto handle = std::coroutine_handle<Promise>::from_promise(*this);
				Task<U> task(handle);
				task.coroutine.promise().last = task.coroutine;
				return task;
			}
		};

		template<>
		struct Promise<void> final : TaskPromiseBase
		{
			void return_void() noexcept {}

			Task<void> get_return_object() noexcept
			{
				auto handle = std::coroutine_handle<Promise>::from_promise(*this);
				Task<void> task(handle);
				task.coroutine.promise().last = task.coroutine;
				return task;
			}
		};

		using promise_type = Promise<T>;
		using handle_type = std::coroutine_handle<promise_type>;

	private:
		friend struct TaskPromiseBase;

		handle_type coroutine;

	public:
		Task() = default;
		Task(handle_type handle) : coroutine(handle) {}

		Task(Task const&) = delete;
		Task& operator=(Task const&) = delete;

		Task(Task&& other) noexcept : coroutine(std::exchange(other.coroutine, nullptr)) {}

		Task& operator=(Task&& other) noexcept
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

		T result() const
		{
			if constexpr (!std::is_void_v<T>)
				return coroutine.promise().result.value();
		}

		bool is_completed() const
		{
			return !coroutine || coroutine.done();
		}

		void resume()
		{
			auto& last = coroutine.promise().last;
			assert(last && !last.done());
			last.resume();
		}
	};

	template<typename T>
	auto TaskPromiseBase::await_transform(Task<T>&& task) noexcept
	{
		struct Awaiter
		{
			Task<T> task;
			TaskPromiseBase* parent;

			bool await_ready() const noexcept { return false; }

			std::coroutine_handle<> await_suspend(std::coroutine_handle<> prev) noexcept
			{
				auto& child = task.coroutine.promise();
				child.prev = prev;
				child.root = parent->root;
				child.root->last = task.coroutine;
				return task.coroutine;
			}

			T await_resume()
			{
				auto& p = task.coroutine.promise();

				if (p.eptr)
					std::rethrow_exception(p.eptr);

				if constexpr (!std::is_void_v<T>)
					return std::move(p.result.value());
			}
		};

		return Awaiter{ std::move(task), this };
	}

	namespace Tasks
	{
		Task<> NextFrame();
		Task<> WaitForSeconds(float seconds);
		Task<> WaitForFrames(int count);
		Task<> WaitWhile(std::function<bool()> condition);
	}
}
