#pragma once

#include <functional>
#include <core/clock.h>

namespace Common
{
	class TimestepFixer
	{
	public:
		using Callback = std::function<void(Clock::Duration)>;

	public:
		void execute(Clock::Duration dTime);
		void execute();

	public:
		auto getTimestep() const { return mTimestep; }
		void setTimestep(Clock::Duration value) { mTimestep = value; }

		void setCallback(Callback value) { mCallback = value; }

		bool getForceTimeCompletion() const { return mForceTimeCompletion; }
		void setForceTimeCompletion(bool value) { mForceTimeCompletion = value; }

	private:
		Clock::Duration mTimestep = Clock::FromSeconds(1.0f / 120.0f);
		Callback mCallback = nullptr;
		bool mForceTimeCompletion = true;

	private:
		Clock::Duration mTimeAccumulator = Clock::Duration::zero();
	};
}