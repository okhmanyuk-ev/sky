#pragma once

#include <list>
#include <optional>
#include <functional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <sky/scheduler.h>
#include <sky/dispatcher.h>
#include <common/easing.h>
#include <sky/clock.h>

#ifdef EMSCRIPTEN
namespace polyfill
{
	template <class Sig> class move_only_function;

	template <class R, class... Args>
	class move_only_function<R(Args...)>
	{
		struct base
		{
			virtual ~base() = default;
			virtual R call(Args... args) = 0;
		};

		template <class F>
		struct impl final : base
		{
			F f;
			explicit impl(F&& fn) : f(std::move(fn)) {}
			R call(Args... args) override
			{
				return std::invoke(f, std::forward<Args>(args)...);
			}
		};

		std::unique_ptr<base> ptr_;

	public:
		move_only_function() noexcept = default;
		move_only_function(std::nullptr_t) noexcept {}

		template <class F,
				  class D = std::decay_t<F>,
				  class = std::enable_if_t<
					  !std::is_same_v<D, move_only_function> &&
					  std::is_invocable_r_v<R, D&, Args...>>>
		move_only_function(F&& f)
			: ptr_(std::make_unique<impl<D>>(std::forward<F>(f))) {}

		move_only_function(move_only_function&&) noexcept = default;
		move_only_function& operator=(move_only_function&&) noexcept = default;

		move_only_function(const move_only_function&) = delete;
		move_only_function& operator=(const move_only_function&) = delete;

		explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

		R operator()(Args... args)
		{
			return ptr_->call(std::forward<Args>(args)...);
		}
	};
}
#endif

namespace sky
{
	class Action
	{
	public:
		enum class Result
		{
			Continue,
			Finished
		};

		Action(Action&&) noexcept = default;
		Action& operator=(Action&&) noexcept = default;
		Action(const Action&) = delete;
		Action& operator=(const Action&) = delete;

		template <std::invocable<sky::Duration> Func>
			requires std::same_as<std::invoke_result_t<Func, sky::Duration>, Result> && (!std::same_as<std::remove_cvref_t<Func>, Action>)
		Action(Func&& func) : mFunc(std::forward<Func>(func))
		{
		}

		template <std::invocable Func>
			requires std::same_as<std::invoke_result_t<Func>, Result>
		Action(Func&& func) :
			Action([func = std::forward<Func>(func)](auto dTime) mutable {
				return func();
			})
		{
		}

		template <std::invocable Func>
			requires std::same_as<std::invoke_result_t<Func>, void>
		Action(Func&& func) :
			Action([func = std::forward<Func>(func)](auto dTime) mutable {
				func();
				return Result::Finished;
			})
		{
		}

		template <std::invocable Func>
			requires std::convertible_to<std::invoke_result_t<Func>, std::tuple<Result, std::optional<Action>>>
		Action(Func&& func) :
			Action([func = std::forward<Func>(func),
				_status = std::optional<Result>{ std::nullopt },
				_action = std::optional<Action>{ std::nullopt }
			](auto delta) mutable {
				if (!_status.has_value())
					std::tie(_status, _action) = func();

				auto status = Result::Finished;

				if (_action)
					status = _action.value()(delta);

				if (status == Result::Continue)
					return Result::Continue;

				if (_status == Result::Finished)
					return Result::Finished;

				_status.reset();
				return Result::Continue;
			})
		{
		}

		template <std::invocable Func>
			requires (!std::is_void_v<std::invoke_result_t<Func>>) &&
				std::convertible_to<std::invoke_result_t<Func>, std::optional<Action>>
		Action(Func&& func) :
			Action([func = std::forward<Func>(func)] mutable -> std::tuple<Result, std::optional<Action>> {
				return { Result::Finished, func() };
			})
		{
		}

		Action(Task<>&& task);

		Result operator()(sky::Duration dTime);

	private:
#ifdef EMSCRIPTEN
		polyfill::move_only_function<Result(sky::Duration)> mFunc;
#else
		std::move_only_function<Result(sky::Duration)> mFunc;
#endif
	};

	class ActionsPlayer
	{
	public:
		void update(sky::Duration delta);
		void add(Action action);
		void clear();
		bool hasActions() const;

	private:
		std::list<Action> mActions;
	};

	namespace Actions
	{
		Action Sequence(std::list<Action> actions);
		Action Concurrent(std::list<Action> actions);
		Action Race(std::list<Action> actions);
		Action RepeatInfinite(std::function<std::optional<Action>()> action);

		Action ExecuteInfinite(std::function<void(sky::Duration delta)> callback);
		Action ExecuteInfinite(std::function<void()> callback);
		Action ExecuteInfiniteGlobal(std::function<void()> callback);

		Action Wait(float duration);

		// will wait while returning true
		Action Wait(std::function<bool(sky::Duration delta)> while_callback);
		Action Wait(std::function<bool()> while_callback);

		// will wait while flag is true
		Action Wait(bool& while_flag);
		Action WaitGlobalFrame();

		Action Delayed(float duration, Action action);

		// will wait while returning true
		Action Delayed(std::function<bool()> while_callback, Action action);

		// will wait while flag is true
		Action Delayed(bool& while_flag, Action action);

		Action Timeout(float duration, Action action);
		Action Breakable(std::function<bool()> while_callback, Action action);

		Action Pausable(std::function<bool()> run_callback, Action action);

		using EasingFunction = std::function<float(float)>;

		template <typename T, typename Func>
			requires std::is_invocable_r_v<void, Func, T>
		Action Interpolate(T start, T dest, float duration, EasingFunction easing, Func callback)
		{
			return [start, dest, duration, easing, callback, passed = 0.0f](auto delta) mutable {
				passed += sky::ToSeconds(delta);
				if (passed >= duration)
				{
					callback(dest);
					return Action::Result::Finished;
				}
				callback(glm::lerp(start, dest, easing(passed / duration)));
				return Action::Result::Continue;
			};
		}

		template<typename T>
		concept Property = requires {
			typename T::Type;
			typename T::Object;
			{ T::GetValue(std::declval<typename T::Object>()) } -> std::convertible_to<typename T::Type>;
			{ T::SetValue(std::declval<typename T::Object>(), std::declval<typename T::Type>()) } -> std::same_as<void>;
		};

		template <Property T>
		Action Interpolate(typename T::Object object, const typename T::Type& start, const typename T::Type& dest,
			float duration, EasingFunction easing = Easing::Linear)
		{
			return Interpolate(start, dest, duration, easing, [object](const auto& value) {
				T::SetValue(object, value);
			});
		}

		template <Property T>
		Action Interpolate(typename T::Object object, const typename T::Type& dest, float duration,
			EasingFunction easing = Easing::Linear)
		{
			return [object, dest, duration, easing] {
				return Interpolate<T>(object, T::GetValue(object), dest, duration, easing);
			};
		}

		template <typename T>
		Action Interpolate(T start, T dest, float duration, T& value, EasingFunction easing = Easing::Linear)
		{
			return Interpolate(start, dest, duration, easing, [&value](T _value) {
				value = _value;
			});
		}

		template <typename T>
		Action Interpolate(T dest, float duration, T& value, EasingFunction easing = Easing::Linear)
		{
			return [dest, duration, &value, easing] {
				return Interpolate(value, dest, duration, value, easing);
			};
		}

		template <typename...Args>
		Action Sequence(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Sequence(std::move(actions));
		}

		template <typename...Args>
		Action Concurrent(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Concurrent(std::move(actions));
		}

		template <typename...Args>
		Action Race(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Race(std::move(actions));
		}
	}
}