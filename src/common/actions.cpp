#include "actions.h"

#include <cassert>

#include <console/device.h>

using namespace Actions;

// parallel action

Parallel::Parallel(Awaiting awaitingType) : mAwaitingType(awaitingType)
{
	//
}

Action::Status Parallel::frame()
{
	auto it = mActions.begin();
	while (it != mActions.end())
	{
		auto& action = *it;

		if (!action)
			it = mActions.erase(it);
		else if (action->frame() == Status::Continue)
			++it;
		else if (mAwaitingType == Awaiting::Any)
			return Status::Finished;
		else
			it = mActions.erase(it);
	}

	if (mActions.empty())
		return Status::Finished;

	return Status::Continue;
}

void Parallel::add(std::unique_ptr<Action> action)
{
	mActions.push_back(std::move(action));
}

void Parallel::clear()
{
	mActions.clear();
}

// sequence action

Action::Status Sequence::frame()
{
	if (mActions.empty())
		return Status::Finished;

	auto& action = mActions.front();

	if (!action || action->frame() == Status::Finished)
		mActions.pop_front();

	return mActions.empty() ? Status::Finished : Status::Continue;
}

void Sequence::add(std::unique_ptr<Action> action, Origin origin)
{
	if (origin == Origin::Begin)
		mActions.push_front(std::move(action));
	else 
		mActions.push_back(std::move(action));
}

void Sequence::clear()
{
	mActions.clear();
}

// generic action

Generic::Generic(StatusCallback callback) : mCallback(callback)
{
	//
}

Generic::Generic(Type type, Callback callback)
{
	mCallback = [type, callback] {
		if (callback)
			callback();

		return type == Type::One ? Status::Finished : Status::Continue;
	};
}

Action::Status Generic::frame()
{
	assert(mCallback);
	return mCallback();
}

// repeat action

Repeat::Repeat(Callback callback) : mCallback(std::move(callback))
{
	//
}

Action::Status Repeat::frame() 
{
	if (!mStatus.has_value())
		std::tie(mStatus, mAction) = mCallback();

	auto status = Status::Finished;

	if (mAction)
		status = mAction->frame();

	if (status == Status::Continue)
		return Status::Continue;
	
	if (mStatus == Status::Finished)
		return Status::Finished;
		
	mStatus.reset();

	return Status::Continue;
}

// collection

Collection::UAction Collection::Insert(std::function<UAction()> action)
{
	return std::make_unique<Repeat>([action]() -> Repeat::Result {
		return { Action::Status::Finished, action() };
	});
}

Collection::UAction Collection::RepeatInfinite(std::function<UAction()> action)
{
	return std::make_unique<Repeat>([action]() -> Repeat::Result {
		return { Action::Status::Continue, action() };
	});
}

Collection::UAction Collection::Execute(std::function<void()> callback)
{
	return std::make_unique<Generic>(Generic::Type::One, callback);
}

Collection::UAction Collection::ExecuteInfinite(std::function<void()> callback)
{
	return std::make_unique<Generic>(Generic::Type::Infinity, callback);
}

Collection::UAction Collection::Wait(float duration)
{
	return Wait([duration]() mutable {
		duration -= Clock::ToSeconds(FRAME->getTimeDelta());
		return duration > 0.0f;
	});
}

Collection::UAction Collection::Wait(std::function<bool()> while_callback)
{
	return std::make_unique<Generic>([while_callback] {
		if (while_callback())
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

Collection::UAction Collection::Wait(bool& while_flag)
{
	return Wait([&while_flag] {
		return while_flag;
	});
}

Collection::UAction Collection::WaitOneFrame()
{
	return Execute(nullptr);
}

Collection::UAction Collection::Delayed(float duration, UAction action)
{
	return MakeSequence(
		Wait(duration),
		std::move(action)
	);
}

Collection::UAction Collection::Delayed(std::function<bool()> while_callback, UAction action)
{
	return MakeSequence(
		Wait(while_callback),
		std::move(action)
	);
}

Collection::UAction Collection::Delayed(bool& while_flag, UAction action)
{
	return MakeSequence(
		Wait(while_flag),
		std::move(action)
	);
}

// breakable

Collection::UAction Collection::Breakable(float duration, UAction action)
{
	return MakeParallel(Parallel::Awaiting::Any,
		Wait(duration),
		std::move(action)
	);
}

Collection::UAction Collection::Breakable(std::function<bool()> while_callback, UAction action)
{
	return MakeParallel(Parallel::Awaiting::Any,
		Wait(while_callback),
		std::move(action)
	);
}

Collection::UAction Collection::Pausable(std::function<bool()> run_callback, UAction action)
{
	auto player = std::make_shared<GenericActionsPlayer<Parallel>>();
	player->add(std::move(action));

	return std::make_unique<Generic>([run_callback, player]() {
		if (!run_callback())
			return Action::Status::Continue;

		player->update();

		if (player->hasActions())
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

Collection::UAction Collection::Interpolate(float start, float dest, float duration, EasingFunction easing, std::function<void(float)> callback)
{
	return std::make_unique<Generic>([start, dest, duration, easing, callback, passed = 0.0f]() mutable {
		passed += Clock::ToSeconds(FRAME->getTimeDelta());
		if (passed >= duration)
		{
			callback(dest);
			return Action::Status::Finished;
		}
		callback(glm::lerp(start, dest, easing(passed / duration)));
		return Action::Status::Continue;
	});
}

Collection::UAction Collection::Interpolate(const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec2&)> callback)
{
	return Collection::Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Collection::UAction Collection::Interpolate(const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec3&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Collection::UAction Collection::Interpolate(const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec4&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Collection::UAction Collection::Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](float _value) {
		value = _value;
	});
}

Collection::UAction Collection::Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Collection::Interpolate(value, destValue, duration, value, easingFunction);
	});
}

Collection::UAction Collection::Interpolate(const glm::vec3& startValue, const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](const glm::vec3& _value) {
		value = _value;
	});
}

Collection::UAction Collection::Interpolate(const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Interpolate(value, destValue, duration, value, easingFunction);
	});
}

Collection::UAction Collection::Log(const std::string& text)
{
	return Execute([text] {
		LOG(text);
	});
}

// instant player

void Actions::Run(std::unique_ptr<Action> action)
{
	//	FRAME->add([action = std::move(action)] {
	//		return action->frame() == Status::Continue ? FrameSystem::Status::Continue : FrameSystem::Status::Finished;
	//	});

	auto player = std::make_shared<GenericActionsPlayer<Parallel>>();
	player->add(std::move(action));
	FRAME->add([player] {
		player->update();

		if (player->hasActions())
			return Common::FrameSystem::Status::Continue;

		return Common::FrameSystem::Status::Finished;
	});
}
