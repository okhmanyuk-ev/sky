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
		void execute(Clock::Duration dTime, Callback callback);
		void execute(Callback callback);

	public:
		auto getTimestep() const { return mTimestep; }
		void setTimestep(Clock::Duration value) { mTimestep = value; }

		bool getForceTimeCompletion() const { return mForceTimeCompletion; }
		void setForceTimeCompletion(bool value) { mForceTimeCompletion = value; }

		auto isDeltaLimiterEnabled() const { return mDeltaLimiterEnabled; }
		void setDeltaLimiterEnabled(bool value) { mDeltaLimiterEnabled = value; }

		auto getDeltaLimit() const { return mDeltaLimit; }
		void setDeltaLimit(Clock::Duration value) { mDeltaLimit = value; }

		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		Clock::Duration mTimestep = Clock::FromSeconds(1.0f / 120.0f);
		bool mForceTimeCompletion = true;
		bool mDeltaLimiterEnabled = false; // avoid very long executes
		Clock::Duration mDeltaLimit = Clock::FromSeconds(1.0f);
		bool mEnabled = true;

	private:
		Clock::Duration mTimeAccumulator = Clock::Duration::zero();
	};
}