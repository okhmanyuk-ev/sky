#pragma once

#include <sky/updatable.h>
#include <sky/clock.h>
#include <functional>

namespace sky
{
	class Timer final : public sky::Updatable
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