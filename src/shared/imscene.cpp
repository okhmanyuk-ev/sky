#include "imscene.h"

using namespace Shared;

void ImScene::onFrame()
{
	for (const auto& name : mUnusedNodes)
	{
		auto node = mNodes.at(name);
		
		bool dont_kill = mDontKillNodes.contains(node);
		
		if (!dont_kill)
		{
			if (mPreKillActions.contains(node))
			{
				node->runAction(Actions::Collection::MakeSequence(
					std::move(mPreKillActions.at(node)),
					Actions::Collection::Kill(node)
				));
			}
			else
			{
				if (node->hasParent())
				{
					node->getParent()->detach(node);
				}
			}
		}
		mPreKillActions.erase(node);
		mNodes.erase(name);
		mDontKillNodes.erase(node);
	}

	mUnusedNodes.clear();

	for (const auto& [name, node] : mNodes)
	{
		mUnusedNodes.insert(name);
	}

	mTypesCount.clear();
}

void ImScene::setupPreKillAction(std::shared_ptr<Scene::Node> node, Actions::Collection::UAction action)
{
	if (mPreKillActions.contains(node))
		mPreKillActions.erase(node);

	mPreKillActions.insert({ node, std::move(action) });
}

void ImScene::dontKill(std::shared_ptr<Scene::Node> node)
{
	mDontKillNodes.insert(node);
}
