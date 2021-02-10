#pragma once

#include <common/frame_system.h>

namespace Common
{
	class Timer final : public FrameSystem::Frameable
	{
	public:
		using Callback = std::function<void()>;
	
	private:
		void onFrame() override;
	
	public:
		auto getInterval() const { return mInterval; }
		void setInterval(Clock::Duration value) { mInterval = value; }

		auto getPassed() const { return mPassed; }
		void setPassed(Clock::Duration value) { mPassed = value; }

		auto& getCallback() const { return mCallback; }
		void setCallback(Callback value) { mCallback = value; }

	private:
		Clock::Duration mInterval = Clock::FromSeconds(1.0f);
		Clock::Duration mPassed = Clock::Duration::zero();
		Callback mCallback = nullptr;
	};
}