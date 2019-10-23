#pragma once

#include <Core/engine.h>
#include <Core/clock.h>
#include <list>
#include <functional>

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

	private:
		std::list<StatusCallback> mFramers;
		int mFramerateLimit = 0;
		bool mSleepAllowed = true;
		double mTimeScale = 1.0;
		Clock::TimePoint mLastTime = Clock::Now();
		Clock::Duration mTimeDelta = Clock::Duration::zero();
	};

	class FrameSystem::Frameable
	{
	public:
		Frameable();
		virtual ~Frameable();

	public:
		virtual void frame() = 0;

	private:
		std::shared_ptr<bool> mFinished = std::make_shared<bool>(false);
	};

	class FrameSystem::Framer final : public Frameable
	{
	public:
		Framer(Callback callback = nullptr);

	private:
		void frame() override;

	public:
		void setCallback(Callback value) { mCallback = value; }

	private:
		Callback mCallback = nullptr;
	};
}