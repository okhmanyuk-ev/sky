#pragma once

#include <core/engine.h>
#include <core/clock.h>
#include <list>
#include <functional>
#include <mutex>

#define FRAME ENGINE->getSystem<Common::FrameSystem>()

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
		auto getFramerateLimit() const { return mFramerateLimit; }
		void setFramerateLimit(int value) { mFramerateLimit = value; }

		bool isSleepAllowed() const { return mSleepAllowed; }
		void setSleepAllowed(bool value) { mSleepAllowed = value; }

		auto getTimeDelta() const { return mTimeDelta; }

		auto getTimeScale() const { return mTimeScale; }
		void setTimeScale(double value) { mTimeScale = value; }

		auto getFramerate() const { return 1.0 / Clock::ToSeconds<double>(mTimeDelta) * mTimeScale; } // frame count per second
		auto getFramerCount() const { return mFramers.size(); }

		auto getUptime() const { return mUptime; }
		auto getFrameCount() { return mFrameCount; }

		auto getTimeDeltaLimit() const { return mTimeDeltaLimit; }
		void setTimeDeltaLimit(std::optional<Clock::Duration> value) { mTimeDeltaLimit = value; }

		auto isChoked() const { return mChoked; }

	private:
		std::list<StatusCallback> mFramers;
		std::list<Callback> mThreadsafeCallbacks;
		int mFramerateLimit = 0;
		bool mSleepAllowed = true;
		double mTimeScale = 1.0;
		Clock::TimePoint mLastTime = Clock::Now();
		Clock::Duration mTimeDelta = Clock::Duration::zero();
		Clock::Duration mUptime = Clock::Duration::zero();
		std::optional<Clock::Duration> mTimeDeltaLimit; // this can save from animation breaks
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