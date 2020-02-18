#pragma once

#include <string>
#include <list>
#include <Common/timer.h>

#define PROFILER ENGINE->getSystem<Common::ProfilerSystem>()

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
		Common::Timer mTimer;

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
		auto getPercentage() const { return Clock::ToSeconds(mDuration) / 1.0f; }

	private:
		std::list<Node*> mNodes;
		std::string mName;
		Node* mParent = nullptr;
		Clock::TimePoint mBeginTime;
		Clock::TimePoint mEndTime;
		Clock::Duration mDuration = Clock::Duration::zero();
		int mCount = 0;
	};
}