#pragma once

#include <string>
#include <list>
#include <sky/timer.h>
#include <sky/locator.h>
#include <sky/clock.h>

#define PROFILER sky::Locator<Common::ProfilerSystem>::GetService()

namespace Common
{
	class ProfilerSystem
	{
	public:
		class Node;

	public:
		ProfilerSystem();

	public:
		void begin(const std::string& name);
		void end();

	private:
		sky::Timer mTimer;

	public:
		auto& getNodes() { return mPublicNodes; }

	private:
		std::list<Node*> mPublicNodes;
		std::list<Node*> mPrivateNodes;
		Node* mCurrentNode = nullptr;
	};

	class ProfilerSystem::Node
	{
		friend ProfilerSystem;

	public:
		auto getDuration() const { return mDuration / mCount; }
		auto getCount() const { return mCount; }
		const auto& getName() const { return mName; }
		auto& getNodes() { return mNodes; }
		auto getPercentage() const { return sky::ToSeconds(mDuration) / 1.0f; }

	private:
		std::list<Node*> mNodes;
		std::string mName;
		Node* mParent = nullptr;
		sky::TimePoint mBeginTime;
		sky::TimePoint mEndTime;
		sky::Duration mDuration = sky::Duration::zero();
		int mCount = 0;
	};
}