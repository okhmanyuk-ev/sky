#include "actions.h"
#include <cassert>
#include <sky/utils.h>

using namespace sky;

Action::Result Action::operator()(sky::Duration dTime)
{
	return mFunc(dTime);
}

void ActionsPlayer::update(sky::Duration delta)
{
	auto it = mActions.begin();
	while (it != mActions.end())
	{
		if ((*it)(delta) == Action::Result::Continue)
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
	return [actions = std::move(actions)] (auto delta) mutable {
		if (actions.empty())
			return Action::Result::Finished;

		if (actions.front()(delta) == Action::Result::Finished)
			actions.pop_front();

		return actions.empty() ? Action::Result::Finished : Action::Result::Continue;
	};
}

Action Actions::Parallel(std::list<Action> actions)
{
	return [actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			if ((*it)(delta) == Action::Result::Continue)
				++it;
			else
				it = actions.erase(it);
		}

		if (actions.empty())
			return Action::Result::Finished;

		return Action::Result::Continue;
	};
}

Action Actions::Race(std::list<Action> actions)
{
	return [actions = std::move(actions)](auto delta) mutable {
		auto it = actions.begin();
		while (it != actions.end())
		{
			if ((*it)(delta) == Action::Result::Continue)
				++it;
			else
				return Action::Result::Finished;
		}

		if (actions.empty())
			return Action::Result::Finished;

		return Action::Result::Continue;
	};
}

Action Actions::RepeatInfinite(std::function<std::optional<Action>()> action)
{
	return [action] -> std::tuple<Action::Result, std::optional<Action>> {
		return { Action::Result::Continue, action() };
	};
}

Action Actions::ExecuteInfinite(std::function<void(sky::Duration delta)> callback)
{
	return [callback](auto delta) {
		callback(delta);
		return Action::Result::Continue;
	};
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
	auto prev_frame_count = sky::Scheduler::Instance->getFrameCount();

	return ExecuteInfinite([prev_frame_count, callback]() mutable {
		auto frame_count = sky::Scheduler::Instance->getFrameCount();

		if (prev_frame_count == frame_count)
			return;

		prev_frame_count = frame_count;
		callback();
	});
}

Action Actions::Wait()
{
	return [] {};
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
	return [while_callback](auto delta) {
		if (while_callback(delta))
			return Action::Result::Continue;

		return Action::Result::Finished;
	};
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
	auto frame_count = sky::Scheduler::Instance->getFrameCount();

	return Wait([frame_count] {
		return frame_count == sky::Scheduler::Instance->getFrameCount();
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

	return [run_callback, player](auto delta) {
		if (!run_callback())
			return Action::Result::Continue;

		player->update(delta);

		if (player->hasActions())
			return Action::Result::Continue;

		return Action::Result::Finished;
	};
}

Action Actions::Log(const std::string& text)
{
	return [text] {
		sky::Log(text);
	};
}

sky::Action Actions::FromTask(std::function<sky::Task<>()> func)
{
	auto completed = std::make_shared<bool>(false);
	return sky::Actions::Sequence(
		[func, completed] {
			sky::Scheduler::Instance->run([](auto func, auto completed) -> sky::Task<> {
				co_await func();
				*completed = true;
			}(func, completed));
		},
		sky::Actions::Wait([completed] {
			return !*completed;
		})
	);
}
