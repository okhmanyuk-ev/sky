#include "imscene.h"
#include <sky/utils.h>

using namespace Shared;

ImScene::NodeItem::NodeItem(std::shared_ptr<Scene::Node> _node, std::function<void()> _destroy_func) :
	node(_node), destroy_func(_destroy_func)
{
}

void ImScene::onFrame()
{
	if (!mNodeItems.empty())
		sky::Indicator("scene", "im nodes", mNodeItems.size());

	for (const auto& name : mUnusedNodes)
	{
		const auto& item = mNodeItems.at(name);
		item.destroy_func();
		mNodeItems.erase(name);
	}

	mUnusedNodes.clear();

	for (const auto& [name, item] : mNodeItems)
	{
		mUnusedNodes.insert(name);
	}

	mTypesCount.clear();
}

void ImScene::destroyCallback(std::function<void()> func)
{	
	mNodeItems.at(mLastSpawnedKey).destroy_func = func;
}

void ImScene::destroyAction(std::unique_ptr<Actions::Action> action)
{
	auto make_shared_function = [](auto f) {
		return [pf = std::make_shared<std::decay_t<decltype(f)>>(std::forward<decltype(f)>(f))] (auto&&...args)->decltype(auto) {
			return (*pf)(decltype(args)(args)...);
		};
	};

	const auto& item = mNodeItems.at(mLastSpawnedKey);

	auto func = make_shared_function([node = item.node, action = std::move(action)] () mutable {
		node->runAction(Actions::Collection::Sequence(
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

void ImScene::dontKillWhileHaveChilds()
{
	const auto& item = mNodeItems.at(mLastSpawnedKey);

	destroyAction(Actions::Collection::Wait([node = item.node] {
		return node->hasNodes();
	}));
}

void ImScene::showAndHideWithScale()
{
	const auto& item = mNodeItems.at(mLastSpawnedKey);

	if (isFirstCall())
	{
		item.node->setScale(0.0f);
		item.node->runAction(Actions::Collection::ChangeScale(item.node, { 1.0f, 1.0f }, 0.25f, Easing::SinusoidalOut));
	}

	destroyAction(Actions::Collection::ChangeScale(item.node, { 0.0f, 0.0f }, 0.25f, Easing::SinusoidalIn));
}

void ImScene::showWithAlpha(float duration, float dst_alpha)
{
	if (!isFirstCall())
		return;

	const auto& item = mNodeItems.at(mLastSpawnedKey);

	auto color = std::dynamic_pointer_cast<Scene::Color>(item.node);

	if (color == nullptr)
		return;

	color->setAlpha(0.0f);
	item.node->runAction(Actions::Collection::ChangeAlpha(color, dst_alpha, duration, Easing::SinusoidalOut));
}

void ImScene::hideWithAlpha(std::shared_ptr<Scene::Color> color, float duration)
{
	destroyAction(Actions::Collection::Hide(color, duration, Easing::SinusoidalIn));
}

void ImScene::hideWithAlpha(float duration)
{
	const auto& item = mNodeItems.at(mLastSpawnedKey);

	auto color = std::dynamic_pointer_cast<Scene::Color>(item.node);

	if (color == nullptr)
		throw std::runtime_error("hideWithAlpha: last spawned node doesnt have 'Color'");

	hideWithAlpha(color, duration);
}
