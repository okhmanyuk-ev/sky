#pragma once

#include <Core/engine.h>
#include <Common/frame_system.h>

namespace Common
{
	class Interpolator final : public FrameSystem::Frameable
	{
	public:
		using FinishCallback = std::function<void()>;
		using ProcessCallback = std::function<void(float)>;
		using EasingFunction = std::function<float(float p)>;

	private:
		void frame() override;

	public:
		float getValue() const { return mValue; }
		void setValue(float value) { mValue = value; }

		float getStartValue() const { return mStartValue; }
		void setStartValue(float value) { mStartValue = value; }

		float getDestinationValue() const { return mDestinationValue; }
		void setDestinationValue(float value) { mDestinationValue = value; }

		auto getDuration() const { return mDuration; }
		void setDuration(Clock::Duration value) { mDuration = value; }

		auto getPassed() const { return mPassed; }
		void setPassed(Clock::Duration value) { mPassed = value; }

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

		Clock::Duration mDuration = Clock::FromSeconds(1.0f);
		Clock::Duration mPassed = Clock::Duration::zero();

		EasingFunction mEasingFunction = nullptr;
		ProcessCallback mProcessCallback = nullptr; // when value was changed
		FinishCallback mFinishCallback = nullptr;
	};
}