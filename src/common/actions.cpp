#include "actions.h"
#include <cassert>
#include <sky/utils.h>

using namespace Actions;

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

void ActionsPlayer::update(sky::Duration delta)
{
	auto it = mActions.begin();
	while (it != mActions.end())
	{
		auto& action = *it;

		if (!action)
			it = mActions.erase(it);
		else if (action->frame(delta) == Action::Status::Continue)
			++it;
		else
			it = mActions.erase(it);
	}
}

void ActionsPlayer::add(std::unique_ptr<Action> action)
{
	mActions.push_back(std::move(action));
}

void ActionsPlayer::clear()
{
	mActions.clear();
}

bool ActionsPlayer::hasActions() const
{
	return !mActions.empty();
}

Generic::Generic(StatusCallback callback) : mCallback(std::move(callback))
{
}

Generic::Generic(Type type, Callback callback)
{
	mCallback = [type, callback = std::move(callback)](auto delta) mutable {
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

std::unique_ptr<Action> Collection::Sequence(std::list<std::unique_ptr<Action>> actions)
{
	return std::make_unique<Generic>([actions = std::move(actions)] (auto delta) mutable {
		if (actions.empty())
			return Action::Status::Finished;

		auto& action = actions.front();

		if (!action || action->frame(delta) == Action::Status::Finished)
			actions.pop_front();

		return actions.empty() ? Action::Status::Finished : Action::Status::Continue;
	});
}

std::unique_ptr<Action> Collection::Repeat(std::function<std::tuple<Action::Status, std::unique_ptr<Action>>()> callback)
{
	return std::make_unique<Generic>([callback,
		_status = std::optional<Action::Status>{ std::nullopt },
		_action = std::unique_ptr<Action>{ nullptr }
	] (auto delta) mutable {
		if (!_status.has_value())
			std::tie(_status, _action) = callback();
		
		auto status = Action::Status::Finished;
		
		if (_action)
			status = _action->frame(delta);
		
		if (status == Action::Status::Continue)
			return Action::Status::Continue;
		
		if (_status == Action::Status::Finished)
			return Action::Status::Finished;
		
		_status.reset();
		return Action::Status::Continue;
	});
}

std::unique_ptr<Action> Collection::Insert(std::function<std::unique_ptr<Action>()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::unique_ptr<Action>> {
		return { Action::Status::Finished, action() };
	});
}

std::unique_ptr<Action> Collection::RepeatInfinite(std::function<std::unique_ptr<Action>()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::unique_ptr<Action>> {
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
	auto player = std::make_shared<ActionsPlayer>();
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

std::unique_ptr<Action> Collection::Log(const std::string& text)
{
	return Execute([text] {
		sky::Log(text);
	});
}
