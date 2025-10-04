#include "actions.h"
#include <shared/scene_helpers.h> // TODO: we should not include shared from scene

using namespace sky;

Action Actions::ChangePositionByDirection(std::shared_ptr<Scene::Transform> node, const glm::vec2& direction, float speed)
{
	return ExecuteInfinite([node, direction, speed](auto delta) {
		auto dTime = sky::ToSeconds(delta);
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

Action Actions::ChangePositionByDirection(std::shared_ptr<Scene::Transform> node, const glm::vec2& direction, float speed, float duration)
{
	return Timeout(duration, ChangePositionByDirection(node, direction, speed));
}

Action Actions::ChangeColorRecursive(std::shared_ptr<Scene::Node> node, const glm::vec4& start,
	const glm::vec4& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec4& value) {
		Shared::SceneHelpers::RecursiveColorSet(node, value);
	});
}

Action Actions::Hide(std::shared_ptr<Scene::Color> node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

Action Actions::Show(std::shared_ptr<Scene::Color> node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

Action Actions::HideRecursive(std::shared_ptr<Scene::Node> node, float duration,
	EasingFunction easingFunction)
{
	std::list<Action> actions;
	std::function<void(std::shared_ptr<Scene::Node> node)> recursive_fill_func;
	recursive_fill_func = [&](std::shared_ptr<Scene::Node> node){
		for (auto child : node->getNodes())
		{
			recursive_fill_func(child);
		}

		auto color_node = std::dynamic_pointer_cast<Scene::Color>(node);

		if (!color_node)
			return;

		actions.push_back(Hide(color_node, duration, easingFunction));
	};
	recursive_fill_func(node);
	return Actions::Concurrent(std::move(actions));
}

Action Actions::Kill(std::shared_ptr<Scene::Node> node)
{
	return [node] {
		sky::Scheduler::Instance->run([](auto node) -> sky::Task<> {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
			co_return;
		}(node));
	};
}
