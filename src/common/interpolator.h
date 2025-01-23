#pragma once

#include <common/scheduler.h>

namespace Common
{
	class Interpolator final : public Scheduler::Frameable
	{
	public:
		using FinishCallback = std::function<void()>;
		using ProcessCallback = std::function<void(float)>;
		using EasingFunction = std::function<float(float p)>;

	private:
		void onFrame() override;

	public:
		float getValue() const { return mValue; }
		void setValue(float value) { mValue = value; }

		float getStartValue() const { return mStartValue; }
		void setStartValue(float value) { mStartValue = value; }

		float getDestinationValue() const { return mDestinationValue; }
		void setDestinationValue(float value) { mDestinationValue = value; }

		auto getDuration() const { return mDuration; }
		void setDuration(sky::Duration value) { mDuration = value; }

		auto getPassed() const { return mPassed; }
		void setPassed(sky::Duration value) { mPassed = value; }

		auto& getProcessCallback() const { return mProcessCallback; }
		void setProcessCallback(ProcessCallback value) { mProcessCallback = value; }

		auto& getFinishCallback() const { return mFinishCallback; }
		void setFinishCallback(FinishCallback value) { mFinishCallback = value; }

		auto& getEasingFunction() const { return mEasingFunction; }
		void setEasingFunction(EasingFunction value) { mEasingFunction = value; }

	private:
		float mValue = 0.0f;
		float mStartValue = 0.0f;
		float mDestinationValue = 0.0f;

		sky::Duration mDuration = sky::FromSeconds(1.0f);
		sky::Duration mPassed = sky::Duration::zero();

		EasingFunction mEasingFunction = nullptr;
		ProcessCallback mProcessCallback = nullptr; // when value was changed
		FinishCallback mFinishCallback = nullptr;
	};
}