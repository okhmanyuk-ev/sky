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

namespace Actions
{
	class Action
	{
	public:
		enum class Status
		{
			Continue,
			Finished
		};

	public:
		virtual ~Action() = default;
		virtual Status frame(sky::Duration delta) = 0;
	};

	class Parallel : public Action
	{
		static_assert(std::has_virtual_destructor<Action>::value);

	public:
		enum class Awaiting
		{
			All,
			Any
		};

	public:
		Parallel(Awaiting awaitingType = Awaiting::All);

	protected:
		Status frame(sky::Duration delta) override;

	public:
		void add(std::unique_ptr<Action> action);
		void clear();
		bool hasActions() const { return mActions.size() > 0; }

	private:
		std::list<std::unique_ptr<Action>> mActions;
		Awaiting mAwaitingType;
	};

	class Sequence : public Action
	{
		static_assert(std::has_virtual_destructor<Action>::value);

	public:
		enum class Origin
		{
			Begin,
			End
		};

	protected:
		Status frame(sky::Duration delta) override;

	public:
		void add(std::unique_ptr<Action> action, Origin origin = Origin::End);
		void clear();
		bool hasActions() const { return mActions.size() > 0; }

	private:
		std::list<std::unique_ptr<Action>> mActions;
	};

	template <typename T> class ActionsPlayer : public T
	{
		static_assert(std::is_same<T, Sequence>::value || std::is_same<T, Parallel>::value,
			"T must be derived from Sequence or Parallel");
	private:
		sky::Scheduler::Framer mFramer = sky::Scheduler::Framer([this] { T::frame(SCHEDULER->getTimeDelta()); });
	};

	using SequentialActionsPlayer = ActionsPlayer<Sequence>;
	using ParallelActionsPlayer = ActionsPlayer<Parallel>;

	template <typename T> class GenericActionsPlayer : public T
	{
		static_assert(std::is_same<T, Sequence>::value || std::is_same<T, Parallel>::value,
			"T must be derived from Sequence or Parallel");

	public:
		void update(sky::Duration delta) { T::frame(delta); }
	};

	class Generic : public Action
	{
	public:
		using StatusCallback = std::function<Status(sky::Duration)>;
		using Callback = std::function<void(sky::Duration)>;

	public:
		enum class Type
		{
			One,
			Infinity
		};

	public:
		Generic(StatusCallback callback);
		Generic(Type type, Callback callback);

	private:
		Status frame(sky::Duration delta) override;

	private:
		StatusCallback mCallback = nullptr;
	};

	class Repeat : public Action
	{
	public:
		using Result = std::tuple<Action::Status, std::unique_ptr<Action>>;
		using Callback = std::function<Result()>;

	public:
		Repeat(Callback callback);

	private:
		Status frame(sky::Duration delta) override;

	private:
		Callback mCallback;
		std::optional<Action::Status> mStatus;
		std::unique_ptr<Action> mAction = nullptr;
	};

	namespace Collection
	{
		std::unique_ptr<Action> Insert(std::function<std::unique_ptr<Action>()> action);
		std::unique_ptr<Action> RepeatInfinite(std::function<std::unique_ptr<Action>()> action);

		std::unique_ptr<Action> Execute(std::function<void()> callback);
		std::unique_ptr<Action> ExecuteInfinite(std::function<void(sky::Duration delta)> callback);
		std::unique_ptr<Action> ExecuteInfinite(std::function<void()> callback);
		std::unique_ptr<Action> ExecuteInfiniteGlobal(std::function<void()> callback);

		std::unique_ptr<Action> Wait();
		std::unique_ptr<Action> Wait(float duration);

		// will wait while returning true
		std::unique_ptr<Action> Wait(std::function<bool(sky::Duration delta)> while_callback);
		std::unique_ptr<Action> Wait(std::function<bool()> while_callback);

		// will wait while flag is true
		std::unique_ptr<Action> Wait(bool& while_flag);
		std::unique_ptr<Action> WaitGlobalFrame();

		std::unique_ptr<Action> Delayed(float duration, std::unique_ptr<Action> action);

		// will wait while returning true
		std::unique_ptr<Action> Delayed(std::function<bool()> while_callback, std::unique_ptr<Action> action);

		// will wait while flag is true
		std::unique_ptr<Action> Delayed(bool& while_flag, std::unique_ptr<Action> action);

		std::unique_ptr<Action> Breakable(float duration, std::unique_ptr<Action> action);
		std::unique_ptr<Action> Breakable(std::function<bool()> while_callback, std::unique_ptr<Action> action);

		std::unique_ptr<Action> Pausable(std::function<bool()> run_callback, std::unique_ptr<Action> action);
		std::unique_ptr<Action> Log(const std::string& text);

		using EasingFunction = std::function<float(float)>;

		template <typename T, typename Func>
			requires std::is_invocable_r_v<void, Func, T>
		std::unique_ptr<Action> Interpolate(T start, T dest, float duration, EasingFunction easing, Func callback)
		{
			return std::make_unique<Generic>([start, dest, duration, easing, callback, passed = 0.0f](auto delta) mutable {
				passed += sky::ToSeconds(delta);
				if (passed >= duration)
				{
					callback(dest);
					return Action::Status::Finished;
				}
				callback(glm::lerp(start, dest, easing(passed / duration)));
				return Action::Status::Continue;
			});
		}

		template<typename T>
		concept Property = requires {
			typename T::Type;
			typename T::Object;
			{ T::GetValue(std::declval<typename T::Object>()) } -> std::convertible_to<typename T::Type>;
			{ T::SetValue(std::declval<typename T::Object>(), std::declval<typename T::Type>()) } -> std::same_as<void>;
		};

		template <Property T>
		std::unique_ptr<Action> Interpolate(typename T::Object object, const typename T::Type& start, const typename T::Type& dest,
			float duration, EasingFunction easing = Easing::Linear)
		{
			return Interpolate(start, dest, duration, easing, [object](const auto& value) {
				T::SetValue(object, value);
			});
		}

		template <Property T>
		std::unique_ptr<Action> Interpolate(typename T::Object object, const typename T::Type& dest, float duration,
			EasingFunction easing = Easing::Linear)
		{
			return Insert([object, dest, duration, easing] {
				return Interpolate<T>(object, T::GetValue(object), dest, duration, easing);
			});
		}

		template <typename T>
		std::unique_ptr<Action> Interpolate(T start, T dest, float duration, T& value, EasingFunction easing = Easing::Linear)
		{
			return Interpolate(start, dest, duration, easing, [&value](T _value) {
				value = _value;
			});
		}

		template <typename T>
		std::unique_ptr<Action> Interpolate(T dest, float duration, T& value, EasingFunction easing = Easing::Linear)
		{
			return Insert([dest, duration, &value, easing] {
				return Interpolate(value, dest, duration, value, easing);
			});
		}

		template <typename...Args>
		std::unique_ptr<Sequence> MakeSequence(Args&&...args)
		{
			auto seq = std::make_unique<Sequence>();
			(seq->add(std::forward<Args>(args)), ...);
			return seq;
		}

		template <typename...Args>
		std::unique_ptr<Parallel> MakeParallel(Parallel::Awaiting awaitingType, Args&&...args)
		{
			auto parallel = std::make_unique<Parallel>(awaitingType);
			(parallel->add(std::forward<Args>(args)), ...);
			return parallel;
		}

		template <typename...Args>
		std::unique_ptr<Parallel> MakeParallel(Args&&...args)
		{
			return MakeParallel(Parallel::Awaiting::All, std::forward<Args>(args)...);
		}

		template <typename T>
		std::unique_ptr<Action> WaitEvent(typename sky::Listener<T>::Callback onEvent)
		{
			auto event_holder = std::make_shared<std::optional<T>>();

			auto listener = std::make_shared<sky::Listener<T>>();
			listener->setCallback([event_holder](const auto& e) {
				if (event_holder->has_value())
					return;

				*event_holder = e;
			});

			return std::make_unique<Actions::Generic>([event_holder, listener, onEvent](auto delta) {
				if (!event_holder->has_value())
					return Actions::Action::Status::Continue;

				onEvent(event_holder->value());
				return Actions::Action::Status::Finished;
			});
		}
	}
}