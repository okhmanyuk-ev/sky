#include "actions.h"
#include <cassert>
#include <sky/utils.h>

using namespace sky;

Action::Action(Callback callback) : mCallback(std::move(callback))
{
}

Action::Status Action::frame(sky::Duration delta)
{
	return mCallback(delta);
}

void ActionsPlayer::update(sky::Duration delta)
{
	auto it = mActions.begin();
	while (it != mActions.end())
	{
		if (it->frame(delta) == Action::Status::Continue)
			++it;
		else
			it = mActions.erase(it);
	}
}

void ActionsPlayer::add(Action action)
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

Action Actions::Sequence(std::list<Action> actions)
{
	return Action([actions = std::move(actions)] (auto delta) mutable {
		if (actions.empty())
			return Action::Status::Finished;

		if (actions.front().frame(delta) == Action::Status::Finished)
			actions.pop_front();

		return actions.empty() ? Action::Status::Finished : Action::Status::Continue;
	});
}

Action Actions::Parallel(std::list<Action> actions)
{
	return Action([actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			if (it->frame(delta) == Action::Status::Continue)
				++it;
			else
				it = actions.erase(it);
		}

		if (actions.empty())
			return Action::Status::Finished;

		return Action::Status::Continue;
	});
}

Action Actions::Race(std::list<Action> actions)
{
	return Action([actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			if (it->frame(delta) == Action::Status::Continue)
				++it;
			else
				return Action::Status::Finished;
		}

		if (actions.empty())
			return Action::Status::Finished;

		return Action::Status::Continue;
	});
}

Action Actions::Repeat(std::function<std::tuple<Action::Status, std::optional<Action>>()> callback)
{
	return Action([callback,
		_status = std::optional<Action::Status>{ std::nullopt },
		_action = std::optional<Action>{ std::nullopt }
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

Action Actions::Insert(std::function<Action()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::optional<Action>> {
		return { Action::Status::Finished, action() };
	});
}

Action Actions::RepeatInfinite(std::function<std::optional<Action>()> action)
{
	return Repeat([action]() -> std::tuple<Action::Status, std::optional<Action>> {
		return { Action::Status::Continue, action() };
	});
}

Action Actions::Execute(std::function<void()> callback)
{
	return Action([callback](auto delta) {
		if (callback)
			callback();

		return Action::Status::Finished;
	});
}

Action Actions::ExecuteInfinite(std::function<void(sky::Duration delta)> callback)
{
	return Action([callback](auto delta) {
		callback(delta);
		return Action::Status::Continue;
	});
}

Action Actions::ExecuteInfinite(std::function<void()> callback)
{
	return ExecuteInfinite([callback](auto delta) {
		if (callback)
			callback();
	});
}

Action Actions::ExecuteInfiniteGlobal(std::function<void()> callback)
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

Action Actions::Wait()
{
	return Execute(nullptr);
}

Action Actions::Wait(float duration)
{
	return Wait([duration](auto delta) mutable {
		duration -= sky::ToSeconds(delta);
		return duration > 0.0f;
	});
}

Action Actions::Wait(std::function<bool(sky::Duration delta)> while_callback)
{
	return Action([while_callback](auto delta) {
		if (while_callback(delta))
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

Action Actions::Wait(std::function<bool()> while_callback)
{
	return Wait([while_callback](auto delta) {
		return while_callback();
	});
}

Action Actions::Wait(bool& while_flag)
{
	return Wait([&while_flag] {
		return while_flag;
	});
}

Action Actions::WaitGlobalFrame()
{
	auto frame_count = SCHEDULER->getFrameCount();

	return Wait([frame_count] {
		return frame_count == SCHEDULER->getFrameCount();
	});
}

Action Actions::Delayed(float duration, Action action)
{
	return Sequence(
		Wait(duration),
		std::move(action)
	);
}

Action Actions::Delayed(std::function<bool()> while_callback, Action action)
{
	return Sequence(
		Wait(while_callback),
		std::move(action)
	);
}

Action Actions::Delayed(bool& while_flag, Action action)
{
	return Sequence(
		Wait(while_flag),
		std::move(action)
	);
}

Action Actions::Breakable(float duration, Action action)
{
	return Race(
		Wait(duration),
		std::move(action)
	);
}

Action Actions::Breakable(std::function<bool()> while_callback, Action action)
{
	return Race(
		Wait(while_callback),
		std::move(action)
	);
}

Action Actions::Pausable(std::function<bool()> run_callback, Action action)
{
	auto player = std::make_shared<ActionsPlayer>();
	player->add(std::move(action));

	return Action([run_callback, player](auto delta) {
		if (!run_callback())
			return Action::Status::Continue;

		player->update(delta);

		if (player->hasActions())
			return Action::Status::Continue;

		return Action::Status::Finished;
	});
}

Action Actions::Log(const std::string& text)
{
	return Execute([text] {
		sky::Log(text);
	});
}
