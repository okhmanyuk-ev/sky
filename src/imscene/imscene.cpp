#include "imscene.h"

void ImScene::ImScene::onFrame()
{
	for (auto name : mUnusedImNodes)
	{
		auto node = mImNodes.at(name);
		node->getParent()->detach(node); // maybe parent does not exist if parent was deleted
		mImNodes.erase(name);
	}

	mUnusedImNodes.clear();

	for (auto [name, node] : mImNodes)
	{
		mUnusedImNodes.insert(name);
	}
}