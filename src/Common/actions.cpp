#include "actions.h"

#include <cassert>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Common;
using namespace Common::Actions;

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
		if ((*it)->frame() == Status::Continue)
			++it;
		else
		{
			if (mAwaitingType == Awaiting::Any)
				return Status::Finished;
		
			it = mActions.erase(it);
		}
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

	if (mActions.front()->frame() == Status::Finished)
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

	if (mAction.has_value())
		status = mAction.value()->frame();

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

// instant player

void Common::Actions::Run(std::unique_ptr<Action> action)
{
	//	FRAME->add([action = std::move(action)] {
	//		return action->frame() == Status::Continue ? FrameSystem::Status::Continue : FrameSystem::Status::Finished;
	//	});

	auto player = new GenericActionsPlayer<Parallel>();
	player->add(std::move(action));
	FRAME->add([player] {
		player->update();

		if (player->hasActions())
			return FrameSystem::Status::Continue;

		delete player;
		return FrameSystem::Status::Finished;
	});
}
