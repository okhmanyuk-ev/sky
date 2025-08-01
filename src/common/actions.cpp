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

std::unique_ptr<Action> Collection::Log(const std::string& text)
{
	return Execute([text] {
		sky::Log(text);
	});
}
