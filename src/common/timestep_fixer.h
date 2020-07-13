#pragma once

#include <functional>

namespace Common
{
	class TimestepFixer
	{
	public:
		using Callback = std::function<void(float)>;

	public:
		void execute(float dTime);
		void execute();

	public:
		float getTimestep() const { return mTimestep; }
		void setTimestep(float value) { mTimestep = value; }

		void setCallback(Callback value) { mCallback = value; }

		bool getForceTimeCompletion() const { return mForceTimeCompletion; }
		void setForceTimeCompletion(bool value) { mForceTimeCompletion = value; }

	private:
		float mTimestep = 1.0f / 120.0f;
		Callback mCallback = nullptr;
		bool mForceTimeCompletion = true;

	private:
		float mTimeAccumulator = 0.0f;
	};
}