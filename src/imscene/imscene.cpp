#include "imscene.h"

void ImScene::ImScene::onFrame()
{
	for (auto name : mUnusedNodes)
	{
		auto node = mNodes.at(name);
		if (node->hasParent())
		{
			node->getParent()->detach(node);
		}
		mNodes.erase(name);
	}

	mUnusedNodes.clear();

	for (auto [name, node] : mNodes)
	{
		mUnusedNodes.insert(name);
	}
}