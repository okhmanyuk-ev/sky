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

namespace sky
{
	enum class ActionResult
	{
		Continue,
		Finished
	};

	using Action = std::function<ActionResult(sky::Duration)>;

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
		Action Parallel(std::list<Action> actions);
		Action Race(std::list<Action> actions);
		Action Repeat(std::function<std::tuple<ActionResult, std::optional<Action>>()> callback);
		Action Insert(std::function<std::optional<Action>()> action);
		Action RepeatInfinite(std::function<std::optional<Action>()> action);

		Action Execute(std::function<void()> callback);
		Action ExecuteInfinite(std::function<void(sky::Duration delta)> callback);
		Action ExecuteInfinite(std::function<void()> callback);
		Action ExecuteInfiniteGlobal(std::function<void()> callback);

		Action Wait();
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

		Action Breakable(float duration, Action action);
		Action Breakable(std::function<bool()> while_callback, Action action);

		Action Pausable(std::function<bool()> run_callback, Action action);
		Action Log(const std::string& text);

		Action Task(Scheduler::Task task);

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
					return ActionResult::Finished;
				}
				callback(glm::lerp(start, dest, easing(passed / duration)));
				return ActionResult::Continue;
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
			return Insert([object, dest, duration, easing] {
				return Interpolate<T>(object, T::GetValue(object), dest, duration, easing);
			});
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
			return Insert([dest, duration, &value, easing] {
				return Interpolate(value, dest, duration, value, easing);
			});
		}

		template <typename...Args>
		Action Sequence(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Sequence(std::move(actions));
		}

		template <typename...Args>
		Action Parallel(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Parallel(std::move(actions));
		}

		template <typename...Args>
		Action Race(Args&&...args)
		{
			std::list<Action> actions;
			(actions.push_back(std::forward<Args>(args)), ...);
			return Race(std::move(actions));
		}

		template <typename T>
		Action WaitEvent(typename sky::Listener<T>::Callback onEvent)
		{
			auto event_holder = std::make_shared<std::optional<T>>();

			auto listener = std::make_shared<sky::Listener<T>>();
			listener->setCallback([event_holder](const auto& e) {
				if (event_holder->has_value())
					return;

				*event_holder = e;
			});

			return std::make_unique<Action>([event_holder, listener, onEvent](auto delta) {
				if (!event_holder->has_value())
					return ActionResult::Continue;

				onEvent(event_holder->value());
				return ActionResult::Finished;
			});
		}
	}
}