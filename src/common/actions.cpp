#include "actions.h"
#include <cassert>
#include <sky/utils.h>

Actions::Action::Action(StatusCallback callback) : mCallback(std::move(callback))
{
}

Actions::Action::Status Actions::Action::frame(sky::Duration delta)
{
	assert(mCallback);
	return mCallback(delta);
}

void Actions::ActionsPlayer::update(sky::Duration delta)
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

void Actions::ActionsPlayer::add(std::unique_ptr<Action> action)
{
	mActions.push_back(std::move(action));
}

void Actions::ActionsPlayer::clear()
{
	mActions.clear();
}

bool Actions::ActionsPlayer::hasActions() const
{
	return !mActions.empty();
}

std::unique_ptr<Actions::Action> Actions::Sequence(std::list<std::unique_ptr<Action>> actions)
{
	return std::make_unique<Action>([actions = std::move(actions)] (auto delta) mutable {
		if (actions.empty())
			return Action::Status::Finished;

		auto& action = actions.front();

		if (!action || action->frame(delta) == Action::Status::Finished)
			actions.pop_front();

		return actions.empty() ? Action::Status::Finished : Action::Status::Continue;
	});
}

std::unique_ptr<Actions::Action> Actions::Parallel(std::list<std::unique_ptr<Action>> actions)
{
	return std::make_unique<Action>([actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			auto& action = *it;

			if (!action)
				it = actions.erase(it);
			else if (action->frame(delta) == Action::Status::Continue)
				++it;
			else
				it = actions.erase(it);
		}

		if (actions.empty())
			return Action::Status::Finished;

		return Action::Status::Continue;
	});
}

std::unique_ptr<Actions::Action> Actions::Race(std::list<std::unique_ptr<Action>> actions)
{
	return std::make_unique<Action>([actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			auto& action = *it;

			if (!action)
				it = actions.erase(it);
			else if (action->frame(delta) == Action::Status::Continue)
				++it;
			else
				return Action::Status::Finished;
		}

		if (actions.empty())
			return Action::Status::Finished;

		return Action::Status::Continue;
	});
}

std::unique_ptr<Actions::Action> Actions::Repeat(std::function<std::tuple<Action::Status, std::unique_ptr<Action>>()> callback)
{
	return std::make_unique<Action>([callback,
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

std::unique_ptr<Actions::Action> Actions::Insert(std::function<std::unique_ptr<Action>()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::unique_ptr<Action>> {
		return { Action::Status::Finished, action() };
	});
}

std::unique_ptr<Actions::Action> Actions::RepeatInfinite(std::function<std::unique_ptr<Action>()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::unique_ptr<Action>> {
		return { Action::Status::Continue, action() };
	});
}

std::unique_ptr<Actions::Action> Actions::Execute(std::function<void()> callback)
{
	return std::make_unique<Action>([callback](auto delta) {
		if (callback)
			callback();

		return Action::Status::Finished;
	});
}

std::unique_ptr<Actions::Action> Actions::ExecuteInfinite(std::function<void(sky::Duration delta)> callback)
{
	return std::make_unique<Action>([callback](auto delta) {
		callback(delta);
		return Action::Status::Continue;
	});
}

std::unique_ptr<Actions::Action> Actions::ExecuteInfinite(std::function<void()> callback)
{
	return ExecuteInfinite([callback](auto delta) {
		if (callback)
			callback();
	});
}

std::unique_ptr<Actions::Action> Actions::ExecuteInfiniteGlobal(std::function<void()> callback)
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

std::unique_ptr<Actions::Action> Actions::Wait()
{
	return Execute(nullptr);
}

std::unique_ptr<Actions::Action> Actions::Wait(float duration)
{
	return Wait([duration](auto delta) mutable {
		duration -= sky::ToSeconds(delta);
		return duration > 0.0f;
	});
}

std::unique_ptr<Actions::Action> Actions::Wait(std::function<bool(sky::Duration delta)> while_callback)
{
	return std::make_unique<Action>([while_callback](auto delta) {
		if (while_callback(delta))
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

std::unique_ptr<Actions::Action> Actions::Wait(std::function<bool()> while_callback)
{
	return Wait([while_callback](auto delta) {
		return while_callback();
	});
}

std::unique_ptr<Actions::Action> Actions::Wait(bool& while_flag)
{
	return Wait([&while_flag] {
		return while_flag;
	});
}

std::unique_ptr<Actions::Action> Actions::WaitGlobalFrame()
{
	auto frame_count = SCHEDULER->getFrameCount();

	return Wait([frame_count] {
		return frame_count == SCHEDULER->getFrameCount();
	});
}

std::unique_ptr<Actions::Action> Actions::Delayed(float duration, std::unique_ptr<Action> action)
{
	return Sequence(
		Wait(duration),
		std::move(action)
	);
}

std::unique_ptr<Actions::Action> Actions::Delayed(std::function<bool()> while_callback, std::unique_ptr<Action> action)
{
	return Sequence(
		Wait(while_callback),
		std::move(action)
	);
}

std::unique_ptr<Actions::Action> Actions::Delayed(bool& while_flag, std::unique_ptr<Action> action)
{
	return Sequence(
		Wait(while_flag),
		std::move(action)
	);
}

std::unique_ptr<Actions::Action> Actions::Breakable(float duration, std::unique_ptr<Action> action)
{
	return Race(
		Wait(duration),
		std::move(action)
	);
}

std::unique_ptr<Actions::Action> Actions::Breakable(std::function<bool()> while_callback, std::unique_ptr<Action> action)
{
	return Race(
		Wait(while_callback),
		std::move(action)
	);
}

std::unique_ptr<Actions::Action> Actions::Pausable(std::function<bool()> run_callback, std::unique_ptr<Action> action)
{
	auto player = std::make_shared<ActionsPlayer>();
	player->add(std::move(action));

	return std::make_unique<Action>([run_callback, player](auto delta) {
		if (!run_callback())
			return Action::Status::Continue;

		player->update(delta);

		if (player->hasActions())
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

std::unique_ptr<Actions::Action> Actions::Log(const std::string& text)
{
	return Execute([text] {
		sky::Log(text);
	});
}
