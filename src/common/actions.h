#pragma once

#include <common/frame_system.h>

#include <list>
#include <optional>
#include <functional>
#include <common/easing.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

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
		Common::FrameSystem::Framer mFramer = Common::FrameSystem::Framer([this] { T::frame(); });
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
		Status frame() override;

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
		Status frame() override;

	private:
		Callback mCallback;
		std::optional<Action::Status> mStatus;
		std::unique_ptr<Action> mAction = nullptr;
	};

	namespace Factory
	{
		using UAction = std::unique_ptr<Action>;

		UAction Insert(std::function<UAction()> action);
		UAction RepeatInfinite(std::function<UAction()> action);

		UAction Execute(std::function<void()> callback);
		UAction ExecuteInfinite(std::function<void()> callback);

		UAction Wait(float duration);
		UAction Wait(std::function<bool()> while_callback);
		UAction WaitOneFrame();

		UAction Delayed(float duration, UAction action);
		UAction Delayed(std::function<bool()> while_callback, UAction action);

		UAction Breakable(float duration, UAction action);
		UAction Breakable(std::function<bool()> while_callback, UAction action);

		UAction Pausable(std::function<bool()> run_callback, UAction action);

		using EasingFunction = std::function<float(float)>;

		UAction Interpolate(float start, float dest, float duration, EasingFunction easingFunction, std::function<void(float)> callback);
		UAction Interpolate(const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec2&)> callback);
		UAction Interpolate(const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec3&)> callback);
		UAction Interpolate(const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec4&)> callback);

		UAction Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction = Easing::Linear);
		UAction Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction = Easing::Linear);

		UAction Interpolate(const glm::vec3& startValue, const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction = Easing::Linear);
		UAction Interpolate(const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction = Easing::Linear);

		UAction Log(const std::string& text);

		template<class...Args> std::unique_ptr<Sequence> MakeSequence(Args&&...args)
		{
			auto seq = std::make_unique<Sequence>();
			(seq->add(std::forward<Args>(args)), ...);
			return seq;
		}

		template<class...Args> std::unique_ptr<Parallel> MakeParallel(Parallel::Awaiting awaitingType, Args&&...args)
		{
			auto parallel = std::make_unique<Parallel>(awaitingType);
			(parallel->add(std::forward<Args>(args)), ...);
			return parallel;
		}

		template<class...Args> std::unique_ptr<Parallel> MakeParallel(Args&&...args)
		{
			return MakeParallel(Parallel::Awaiting::All, std::forward<Args>(args)...);
		}
	}

	// an instant action player whithout states,
	// will be removed as soon as action completed
	void Run(std::unique_ptr<Action> action);
}