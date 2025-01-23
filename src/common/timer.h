#pragma once

#include <sky/scheduler.h>

namespace Common
{
	class Timer final : public sky::Scheduler::Frameable
	{
	public:
		using Callback = std::function<void()>;

	private:
		void onFrame() override;

	public:
		auto getInterval() const { return mInterval; }
		void setInterval(sky::Duration value) { mInterval = value; }

		auto getPassed() const { return mPassed; }
		void setPassed(sky::Duration value) { mPassed = value; }

		auto& getCallback() const { return mCallback; }
		void setCallback(Callback value) { mCallback = value; }

	private:
		sky::Duration mInterval = sky::FromSeconds(1.0f);
		sky::Duration mPassed = sky::Duration::zero();
		Callback mCallback = nullptr;
	};
}