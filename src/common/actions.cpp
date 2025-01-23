#include "actions.h"
#include <cassert>
#include <sky/utils.h>

using namespace Actions;

// parallel action

Parallel::Parallel(Awaiting awaitingType) : mAwaitingType(awaitingType)
{
}

Action::Status Parallel::frame(sky::Duration delta)
{
	auto it = mActions.begin();
	while (it != mActions.end())
	{
		auto& action = *it;

		if (!action)
			it = mActions.erase(it);
		else if (action->frame(delta) == Status::Continue)
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

Action::Status Sequence::frame(sky::Duration delta)
{
	if (mActions.empty())
		return Status::Finished;

	auto& action = mActions.front();

	if (!action || action->frame(delta) == Status::Finished)
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
}

Generic::Generic(Type type, Callback callback)
{
	mCallback = [type, callback](auto delta) {
		if (callback)
			callback(delta);

		return type == Type::One ? Status::Finished : Status::Continue;
	};
}

Action::Status Generic::frame(sky::Duration delta)
{
	assert(mCallback);
	return mCallback(delta);
}

// repeat action

Repeat::Repeat(Callback callback) : mCallback(std::move(callback))
{
}

Action::Status Repeat::frame(sky::Duration delta)
{
	if (!mStatus.has_value())
		std::tie(mStatus, mAction) = mCallback();

	auto status = Status::Finished;

	if (mAction)
		status = mAction->frame(delta);

	if (status == Status::Continue)
		return Status::Continue;

	if (mStatus == Status::Finished)
		return Status::Finished;

	mStatus.reset();

	return Status::Continue;
}

// collection

std::unique_ptr<Action> Collection::Insert(std::function<std::unique_ptr<Action>()> action)
{
	return std::make_unique<Repeat>([action]() -> Repeat::Result {
		return { Action::Status::Finished, action() };
	});
}

std::unique_ptr<Action> Collection::RepeatInfinite(std::function<std::unique_ptr<Action>()> action)
{
	return std::make_unique<Repeat>([action]() -> Repeat::Result {
		return { Action::Status::Continue, action() };
	});
}

std::unique_ptr<Action> Collection::Execute(std::function<void()> callback)
{
	return std::make_unique<Generic>(Generic::Type::One, [callback](auto delta) {
		if (callback)
			callback();
	});
}

std::unique_ptr<Action> Collection::ExecuteInfinite(std::function<void(sky::Duration delta)> callback)
{
	return std::make_unique<Generic>(Generic::Type::Infinity, callback);
}

std::unique_ptr<Action> Collection::ExecuteInfinite(std::function<void()> callback)
{
	return ExecuteInfinite([callback](auto delta) {
		if (callback)
			callback();
	});
}

std::unique_ptr<Action> Collection::ExecuteInfiniteGlobal(std::function<void()> callback)
{
	auto prev_frame_count = SCHEDULER->getFrameCount();

	return ExecuteInfinite([prev_frame_count, callback]() mutable {
		auto frame_count = SCHEDULER->getFrameCount();

		if (prev_frame_count == frame_count)
			return;

		prev_frame_count = frame_count;
		callback();
	});
}

std::unique_ptr<Action> Collection::Wait()
{
	return Execute(nullptr);
}

std::unique_ptr<Action> Collection::Wait(float duration)
{
	return Wait([duration](auto delta) mutable {
		duration -= sky::ToSeconds(delta);
		return duration > 0.0f;
	});
}

std::unique_ptr<Action> Collection::Wait(std::function<bool(sky::Duration delta)> while_callback)
{
	return std::make_unique<Generic>([while_callback](auto delta) {
		if (while_callback(delta))
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

std::unique_ptr<Action> Collection::Wait(std::function<bool()> while_callback)
{
	return Wait([while_callback](auto delta) {
		return while_callback();
	});
}

std::unique_ptr<Action> Collection::Wait(bool& while_flag)
{
	return Wait([&while_flag] {
		return while_flag;
	});
}

std::unique_ptr<Action> Collection::WaitGlobalFrame()
{
	auto frame_count = SCHEDULER->getFrameCount();

	return Wait([frame_count] {
		return frame_count == SCHEDULER->getFrameCount();
	});
}

std::unique_ptr<Action> Collection::Delayed(float duration, std::unique_ptr<Action> action)
{
	return MakeSequence(
		Wait(duration),
		std::move(action)
	);
}

std::unique_ptr<Action> Collection::Delayed(std::function<bool()> while_callback, std::unique_ptr<Action> action)
{
	return MakeSequence(
		Wait(while_callback),
		std::move(action)
	);
}

std::unique_ptr<Action> Collection::Delayed(bool& while_flag, std::unique_ptr<Action> action)
{
	return MakeSequence(
		Wait(while_flag),
		std::move(action)
	);
}

// breakable

std::unique_ptr<Action> Collection::Breakable(float duration, std::unique_ptr<Action> action)
{
	return MakeParallel(Parallel::Awaiting::Any,
		Wait(duration),
		std::move(action)
	);
}

std::unique_ptr<Action> Collection::Breakable(std::function<bool()> while_callback, std::unique_ptr<Action> action)
{
	return MakeParallel(Parallel::Awaiting::Any,
		Wait(while_callback),
		std::move(action)
	);
}

std::unique_ptr<Action> Collection::Pausable(std::function<bool()> run_callback, std::unique_ptr<Action> action)
{
	auto player = std::make_shared<GenericActionsPlayer<Parallel>>();
	player->add(std::move(action));

	return std::make_unique<Generic>([run_callback, player](auto delta) {
		if (!run_callback())
			return Action::Status::Continue;

		player->update(delta);

		if (player->hasActions())
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

std::unique_ptr<Action> Collection::Interpolate(float start, float dest, float duration, EasingFunction easing, std::function<void(float)> callback)
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

std::unique_ptr<Action> Collection::Interpolate(const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec2&)> callback)
{
	return Collection::Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

std::unique_ptr<Action> Collection::Interpolate(const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec3&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

std::unique_ptr<Action> Collection::Interpolate(const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec4&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

std::unique_ptr<Action> Collection::Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](float _value) {
		value = _value;
	});
}

std::unique_ptr<Action> Collection::Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Collection::Interpolate(value, destValue, duration, value, easingFunction);
	});
}

std::unique_ptr<Action> Collection::Interpolate(const glm::vec3& startValue, const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](const glm::vec3& _value) {
		value = _value;
	});
}

std::unique_ptr<Action> Collection::Interpolate(const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Interpolate(value, destValue, duration, value, easingFunction);
	});
}

std::unique_ptr<Action> Collection::Log(const std::string& text)
{
	return Execute([text] {
		sky::Log(text);
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
	SCHEDULER->add([player] {
		player->update(SCHEDULER->getTimeDelta());

		if (player->hasActions())
			return Common::Scheduler::Status::Continue;

		return Common::Scheduler::Status::Finished;
	});
}
