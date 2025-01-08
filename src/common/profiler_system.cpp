#include "profiler_system.h"

using namespace Common;

ProfilerSystem::ProfilerSystem()
{
	mTimer.setInterval(sky::FromSeconds(1.0f));
	mTimer.setCallback([this] {
		std::function<void(std::list<Node*>)> freeAll = [&freeAll](std::list<Node*> nodes) {
			for (auto node : nodes)
			{
				freeAll(node->getNodes());
				delete node;
			}
		};

		freeAll(mPublicNodes);
		mPublicNodes = mPrivateNodes;
		mPrivateNodes.clear();
	});
}

void ProfilerSystem::begin(const std::string& name)
{
	auto& nodeList = mCurrentNode == nullptr ? mPrivateNodes : mCurrentNode->getNodes();

	Node* node = nullptr;

	for (auto n : nodeList)
	{
		if (n->getName() != name)
			continue;

		node = n;
	}

	if (node == nullptr)
	{
		node = new Node();
		node->mName = name;
		node->mParent = mCurrentNode;
		nodeList.push_back(node);
	}

	node->mCount += 1;
	node->mBeginTime = sky::Now();

	mCurrentNode = node;
}

void ProfilerSystem::end()
{
	mCurrentNode->mEndTime = sky::Now();
	mCurrentNode->mDuration += mCurrentNode->mEndTime - mCurrentNode->mBeginTime;
	mCurrentNode = mCurrentNode->mParent;
}