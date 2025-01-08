#pragma once

#include <functional>
#include <sky/clock.h>

namespace Common
{
	class TimestepFixer
	{
	public:
		using Callback = std::function<void(sky::Duration)>;

	public:
		void execute(sky::Duration dTime, Callback callback);
		void execute(Callback callback);

	public:
		auto getTimestep() const { return mTimestep; }
		void setTimestep(sky::Duration value) { mTimestep = value; }

		bool getForceTimeCompletion() const { return mForceTimeCompletion; }
		void setForceTimeCompletion(bool value) { mForceTimeCompletion = value; }

		auto isSkipLongFrames() const { return mSkipLongFrames; }
		void setSkipLongFrames(bool value) { mSkipLongFrames = value; }

		auto getLongFrameDuration() const { return mLongFrameDuration; }
		void setLongFrameDuration(sky::Duration value) { mLongFrameDuration = value; }

		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		sky::Duration mTimestep = sky::FromSeconds(1.0f / 120.0f);
		bool mForceTimeCompletion = true;
		bool mSkipLongFrames = false;
		sky::Duration mLongFrameDuration = sky::FromSeconds(1.0f);
		bool mEnabled = true;

	private:
		sky::Duration mTimeAccumulator = sky::Duration::zero();
	};
}