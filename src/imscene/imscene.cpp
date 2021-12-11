#include "imscene.h"

void ImScene::ImScene::onFrame()
{
	for (auto name : mUnusedNodes)
	{
		auto node = mNodes.at(name);
		node->getParent()->detach(node); // maybe parent does not exist if parent was deleted
		mNodes.erase(name);
	}

	mUnusedNodes.clear();

	for (auto [name, node] : mNodes)
	{
		mUnusedNodes.insert(name);
	}
}