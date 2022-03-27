#include "imscene.h"

using namespace Shared;

void ImScene::onFrame()
{
	for (const auto& name : mUnusedNodes)
	{
		auto node = mNodes.at(name);
		mDestroyCallbacks.at(node)();
		mDestroyCallbacks.erase(node);
		mNodes.erase(name);
	}

	mUnusedNodes.clear();

	for (const auto& [name, node] : mNodes)
	{
		mUnusedNodes.insert(name);
	}

	mTypesCount.clear();
}

void ImScene::destroyCallback(std::shared_ptr<Scene::Node> node, std::function<void()> func)
{
	if (mDestroyCallbacks.contains(node))
		mDestroyCallbacks.erase(node);

	mDestroyCallbacks.insert({ node, func });
}

void ImScene::destroyAction(std::shared_ptr<Scene::Node> node, Actions::Collection::UAction action)
{
	auto make_shared_function = [](auto f) {
		return [pf = std::make_shared<std::decay_t<decltype(f)>>(std::forward<decltype(f)>(f))] (auto&&...args)->decltype(auto) {
			return (*pf)(decltype(args)(args)...);
		};
	};

	auto func = make_shared_function([node, action = std::move(action)] () mutable {
		node->runAction(Actions::Collection::MakeSequence(
			std::move(action),
			Actions::Collection::Kill(node)
		));
	});

	destroyCallback(node, func);
}

void ImScene::dontKill(std::shared_ptr<Scene::Node> node)
{
	destroyCallback(node, [] {});
}
