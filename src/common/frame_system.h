#pragma once

#include <list>
#include <functional>
#include <mutex>
#include <sky/locator.h>
#include <sky/clock.h>
#include <sky/console.h>

#define FRAME sky::Locator<Common::FrameSystem>::GetService()

namespace Common
{
	class FrameSystem
	{
	public:
		class Frameable;
		class Framer;

	public:
		enum class Status
		{
			Finished,
			Continue
		};

	public:
		using Callback = std::function<void()>;
		using StatusCallback = std::function<Status()>;

	public:
		void frame();

	public:
		void add(StatusCallback callback);
		void addInfinity(Callback callback);
		void addOne(Callback callback);
		void addOneThreadsafe(Callback callback);

	public:
		int getFramerateLimit() const { return mFramerateLimit; }
		void setFramerateLimit(int value) { mFramerateLimit = value; }

		bool isSleepAllowed() const { return mSleepAllowed; }
		void setSleepAllowed(bool value) { mSleepAllowed = value; }

		auto getTimeDelta() const { return mTimeDelta; }

		float getTimeScale() const { return mTimeScale; }
		void setTimeScale(float value) { mTimeScale = value; }

		auto getFramerate() const { return 1.0f / sky::ToSeconds(mTimeDelta) * mTimeScale; } // frame count per second
		auto getFramerCount() const { return mFramers.size(); }

		auto getUptime() const { return mUptime; }
		auto getFrameCount() { return mFrameCount; }

		auto getTimeDeltaLimit() const { return mTimeDeltaLimit; }
		void setTimeDeltaLimit(std::optional<sky::Duration> value) { mTimeDeltaLimit = value; }

		auto isChoked() const { return mChoked; }

	private:
		std::list<StatusCallback> mFramers;
		std::list<Callback> mThreadsafeCallbacks;
		sky::CVarInt mFramerateLimit = sky::CVarInt("sys_framerate", 0, "limit of fps");
		sky::CVarBool mSleepAllowed = sky::CVarBool("sys_sleep", true, "cpu saving between frames");
		sky::CVarFloat mTimeScale = sky::CVarFloat("sys_timescale", 1.0f, "time delta multiplier");
		sky::TimePoint mLastTime = sky::Now();
		sky::Duration mTimeDelta = sky::Duration::zero();
		sky::Duration mUptime = sky::Duration::zero();
		std::optional<sky::Duration> mTimeDeltaLimit; // this can save from animation breaks
		uint64_t mFrameCount = 0;
		std::mutex mMutex;
		bool mChoked = false;
	};

	class FrameSystem::Frameable
	{
	public:
		Frameable();
		~Frameable();

	public:
		virtual void onFrame() = 0;

	private:
		std::shared_ptr<bool> mFinished = std::make_shared<bool>(false);
	};

	class FrameSystem::Framer final : public Frameable
	{
	public:
		Framer(Callback callback = nullptr);

	private:
		void onFrame() override;

	public:
		void setCallback(Callback value) { mCallback = value; }

	private:
		Callback mCallback = nullptr;
	};
}