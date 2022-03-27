#include "imscene.h"

using namespace Shared;

void ImScene::onFrame()
{
	for (const auto& name : mUnusedNodes)
	{
		auto node = mNodes.at(name);	
		node->runAction(std::move(mDestroyActions.at(node)));
		mDestroyActions.erase(node);
		mNodes.erase(name);
	}

	mUnusedNodes.clear();

	for (const auto& [name, node] : mNodes)
	{
		mUnusedNodes.insert(name);
	}

	mTypesCount.clear();
}

void ImScene::destroyAction(std::shared_ptr<Scene::Node> node, Actions::Collection::UAction _action)
{
	if (mDestroyActions.contains(node))
		mDestroyActions.erase(node);

	auto action = Actions::Collection::MakeSequence(
		std::move(_action),
		Actions::Collection::Kill(node)
	);

	mDestroyActions.insert({ node, std::move(action) });
}

void ImScene::dontKill(std::shared_ptr<Scene::Node> node)
{
	if (mDestroyActions.contains(node))
		mDestroyActions.erase(node);

	mDestroyActions.insert({ node, nullptr });
}
