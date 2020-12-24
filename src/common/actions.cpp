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

// wait action

Wait::Wait(Clock::Duration duration) : mDuration(duration)
{
	//
}

Action::Status Wait::frame()
{
	mPassed += FRAME->getTimeDelta();
	return mPassed >= mDuration ? Status::Finished : Status::Continue;
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

// interpolate action

Interpolate::Interpolate(float startValue, float destValue, Clock::Duration duration,
	EasingFunction easingFunction, ProcessCallback processCallback) :
	mStartValue(startValue),
	mDestinationValue(destValue),
	mDuration(duration),
	mEasingFunction(easingFunction),
	mProcessCallback(processCallback)
{
	//
}

Action::Status Interpolate::frame()
{
	mPassed += FRAME->getTimeDelta();

	if (mPassed >= mDuration)
	{
		mProcessCallback(mDestinationValue);
		return Status::Finished;
	}

	float p = Clock::ToSeconds(mPassed);
	float d = Clock::ToSeconds(mDuration);
	
	mProcessCallback(glm::lerp(mStartValue, mDestinationValue, mEasingFunction(p / d)));
	
	return Status::Continue;
}

// factory

Factory::UAction Factory::Insert(std::function<UAction()> action)
{
	return std::make_unique<Actions::Repeat>([action]() -> Actions::Repeat::Result {
		return { Actions::Action::Status::Finished, action() };
	});
}

Factory::UAction Factory::RepeatInfinite(std::function<UAction()> action)
{
	return std::make_unique<Actions::Repeat>([action]() -> Actions::Repeat::Result {
		return { Actions::Action::Status::Continue, action() };
	});
}

Factory::UAction Factory::Execute(std::function<void()> callback)
{
	return std::make_unique<Actions::Generic>(Actions::Generic::Type::One, callback);
}

Factory::UAction Factory::ExecuteInfinite(std::function<void()> callback)
{
	return std::make_unique<Actions::Generic>(Actions::Generic::Type::Infinity, callback);
}

Factory::UAction Factory::Wait(float duration)
{
	return Wait([duration]() mutable {
		duration -= Clock::ToSeconds(FRAME->getTimeDelta());
		return duration > 0.0f;
	});
}

Factory::UAction Factory::Wait(std::function<bool()> while_callback)
{
	return std::make_unique<Actions::Generic>([while_callback] {
		if (while_callback())
			return Actions::Action::Status::Continue;

		return Actions::Action::Status::Finished;
	});
}

Factory::UAction Factory::WaitOneFrame()
{
	return Execute(nullptr);
}

Factory::UAction Factory::Delayed(float duration, UAction action)
{
	return MakeSequence(
		Wait(duration),
		std::move(action)
	);
}

Factory::UAction Factory::Delayed(std::function<bool()> while_callback, UAction action)
{
	return MakeSequence(
		Wait(while_callback),
		std::move(action)
	);
}

// breakable

Factory::UAction Factory::Breakable(float duration, UAction action)
{
	return MakeParallel(Actions::Parallel::Awaiting::Any,
		Wait(duration),
		std::move(action)
	);
}

Factory::UAction Factory::Breakable(std::function<bool()> while_callback, UAction action)
{
	return MakeParallel(Actions::Parallel::Awaiting::Any,
		Wait(while_callback),
		std::move(action)
	);
}

Factory::UAction Factory::Pausable(std::function<bool()> run_callback, UAction action)
{
	auto player = std::make_shared<Actions::GenericActionsPlayer<Actions::Parallel>>();
	player->add(std::move(action));

	return std::make_unique<Actions::Generic>([run_callback, player]() {
		if (!run_callback())
			return Actions::Action::Status::Continue;

		player->update();

		if (player->hasActions())
			return Actions::Action::Status::Continue;

		return Actions::Action::Status::Finished;
	});
}

Factory::UAction Factory::Interpolate(float start, float dest, float duration, EasingFunction easingFunction, std::function<void(float)> callback)
{
	return std::make_unique<Actions::Interpolate>(start, dest, Clock::FromSeconds(duration), easingFunction, callback);
}

Factory::UAction Factory::Interpolate(const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec2&)> callback)
{
	return Factory::Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Factory::UAction Factory::Interpolate(const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec3&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Factory::UAction Factory::Interpolate(const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec4&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

Factory::UAction Factory::Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](float _value) {
		value = _value;
	});
}

Factory::UAction Factory::Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Factory::Interpolate(value, destValue, duration, value, easingFunction);
	});
}

Factory::UAction Factory::Interpolate(const glm::vec3& startValue, const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](const glm::vec3& _value) {
		value = _value;
	});
}

Factory::UAction Factory::Interpolate(const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Interpolate(value, destValue, duration, value, easingFunction);
	});
}

Factory::UAction Factory::Log(const std::string& text)
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
