#include "imscene.h"
#include <shared/stats_system.h>

using namespace Shared;

void ImScene::onFrame()
{
	if (!mNodes.empty())
		STATS_INDICATE_GROUP("scene", "im nodes", mNodes.size());

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
	mLastSpawn = nullptr;
}

void ImScene::destroyCallback(std::function<void()> func)
{
	if (mDestroyCallbacks.contains(mLastSpawn))
		mDestroyCallbacks.erase(mLastSpawn);

	mDestroyCallbacks.insert({ mLastSpawn, func });
}

void ImScene::destroyAction(Actions::Collection::UAction action)
{
	auto make_shared_function = [](auto f) {
		return [pf = std::make_shared<std::decay_t<decltype(f)>>(std::forward<decltype(f)>(f))] (auto&&...args)->decltype(auto) {
			return (*pf)(decltype(args)(args)...);
		};
	};

	auto func = make_shared_function([node = mLastSpawn, action = std::move(action)] () mutable {
		node->runAction(Actions::Collection::MakeSequence(
			std::move(action),
			Actions::Collection::Kill(node)
		));
	});

	destroyCallback(func);
}

void ImScene::dontKill()
{
	destroyCallback([] {});
}

void ImScene::dontKillUntilHaveChilds()
{
	destroyAction(Actions::Collection::Wait([node = mLastSpawn] {
		return node->hasNodes();
	}));
}

void ImScene::showAndHideWithScale()
{
	if (IMSCENE->justAllocated())
	{
		mLastSpawn->setScale(0.0f);
		mLastSpawn->runAction(Actions::Collection::ChangeScale(mLastSpawn, { 1.0f, 1.0f }, 0.25f, Easing::SinusoidalOut));
	}

	IMSCENE->destroyAction(Actions::Collection::ChangeScale(mLastSpawn, { 0.0f, 0.0f }, 0.25f, Easing::SinusoidalIn));
}

