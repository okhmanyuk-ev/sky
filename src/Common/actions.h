#pragma once

#include <Common/frame_system.h>

#include <list>
#include <optional>
#include <functional>

namespace Common::Actions
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
		virtual Status frame() = 0;
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
		Status frame() override;

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
		Status frame() override;

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
		FrameSystem::Framer mFramer = FrameSystem::Framer([this] { T::frame(); });
	};

	using SequentialActionsPlayer = ActionsPlayer<Sequence>;
	using ParallelActionsPlayer = ActionsPlayer<Parallel>;

	template <typename T> class GenericActionsPlayer : public T 
	{
		static_assert(std::is_same<T, Sequence>::value || std::is_same<T, Parallel>::value,
			"T must be derived from Sequence or Parallel");

	public:
		void update() { T::frame(); }
	};

	class Generic : public Action
	{
	public:
		using StatusCallback = std::function<Status()>;
		using Callback = std::function<void()>;
		enum class Type {
			One,
			Infinity
		};

	public:
		Generic(StatusCallback callback);
		Generic(Type type, Callback callback);

	private:
		Status frame() override;

	private:
		StatusCallback mCallback = nullptr;
	};

	class Wait : public Action
	{
	public:
		Wait(Clock::Duration duration);
		Wait(float duration_sec);

	private:
		Status frame() override;
	
	private:
		Clock::Duration mDuration = Clock::Duration::zero();
		Clock::Duration mPassed = Clock::Duration::zero();
	};

	class Repeat : public Action
	{
	public:
		using OptionalAction = std::optional<std::unique_ptr<Action>>;
		
		using Result = std::pair<Action::Status, OptionalAction>;
		using Callback = std::function<Result()>;
		
		using InfiniteResult = OptionalAction;
		using InifinteCallback = std::function<OptionalAction()>;

	public:
		Repeat(Callback callback);
		Repeat(InifinteCallback callback);

	private:
		Status frame() override;

	private:
		Callback mCallback;
		std::optional<Action::Status> mStatus;
		std::optional<std::unique_ptr<Action>> mAction;
	};

	class Interpolate : public Action
	{
	public:
		using ProcessCallback = std::function<void(float)>;
		using EasingFunction = std::function<float(float p)>;

	public:
		Interpolate(float startValue, float destValue, Clock::Duration duration,
			EasingFunction easingFunction, ProcessCallback processCallback);

		Interpolate(float startValue, float destValue,
			float duration, EasingFunction easingFunction, ProcessCallback processCallback);

	private:
		Status frame() override;
	
	private:
		Clock::Duration mDuration = Clock::Duration::zero();
		Clock::Duration mPassed = Clock::Duration::zero();

		float mStartValue = 0.0f;
		float mDestinationValue = 0.0f;

		EasingFunction mEasingFunction = nullptr;
		ProcessCallback mProcessCallback = nullptr;
	};

	class Insert : public Action
	{
	public:
		using Callback = std::function<std::optional<std::unique_ptr<Action>>()>;

	public:
		Insert(Callback callback);

	private:
		Status frame() override;

	private:
		Callback mCallback = nullptr;
		std::optional<std::unique_ptr<Action>> mAction;
	};

	// an instant action player whithout states,
	// will be removed as soon as action completed
	void Run(std::unique_ptr<Action> action);
}