#include "scene.h"

Scene::Scene::Scene()
{
	mRoot->setStretch({ 1.0f, 1.0f });
}

Scene::Scene::~Scene()
{
	//
}

void Scene::Scene::recursiveNodeUpdate(const std::shared_ptr<Node>& node)
{
	if (!node->isEnabled())
		return;

	node->update();

	for (const auto& _node : node->getNodes())
		recursiveNodeUpdate(_node);
}

void Scene::Scene::recursiveNodeDraw(const std::shared_ptr<Node>& node)
{
	if (!node->isEnabled())
		return;

	node->beginRender();
	node->draw();

	for (const auto& _node : node->getNodes())
		recursiveNodeDraw(_node);

	node->endRender();
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getTouchableNodes(const std::shared_ptr<Node>& node, const glm::vec2& pos)
{
	if (!node->isEnabled())
		return { };

	if (!node->isInteractions())
		return { };

	if (!node->interactTest(node->unproject(pos)))
		return { };

	const auto& nodes = node->getNodes();

	std::list<std::shared_ptr<Node>> result;

	for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
	{
		for (auto node : getTouchableNodes(*it, pos))
		{
			result.push_back(node);
		}
	}

	if (!node->isTouchable())
		return result;

	if (!node->hitTest(node->unproject(pos)))
		return result;

	result.push_back(node);

	return result;
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getTouchableNodes(const glm::vec2& pos)
{
	return getTouchableNodes(mRoot, pos);
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getNodes(const std::shared_ptr<Node>& node, const glm::vec2& pos)
{
	if (!node->isEnabled())
		return { };

	if (!node->interactTest(node->unproject(pos)))
		return { };

	auto& nodes = node->getNodes();

	std::list<std::shared_ptr<Node>> result;

	for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
	{
		for (auto node : getNodes(*it, pos))
		{
			result.push_back(node);
		}
	}

	if (!node->hitTest(node->unproject(pos)))
		return result;

	result.push_back(node);

	return result;
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getNodes(const glm::vec2& pos)
{
	return getNodes(mRoot, pos);
}

void Scene::Scene::frame()
{
	updateTransformations();
	GRAPHICS->beginOrtho();
	recursiveNodeDraw(mRoot);
	GRAPHICS->end();
}

void Scene::Scene::updateTransformations()
{
	recursiveNodeUpdate(mRoot);
}

size_t Scene::Scene::getNodesCount(std::shared_ptr<Node> node) const
{
	if (node == nullptr)
		return getNodesCount(mRoot);
	
	size_t result = 1;

	for (const auto& _node : node->getNodes())
	{
		result += getNodesCount(_node);
	}

	return result;
}

void Scene::Scene::event(const Platform::Mouse::Event& e)
{
	if (e.type == Platform::Mouse::Event::Type::ButtonDown)
	{
		event(Platform::Touch::Event({ Platform::Touch::Event::Type::Begin, e.x, e.y }));
	}
	else if (e.type == Platform::Mouse::Event::Type::Move)
	{
		event(Platform::Touch::Event({ Platform::Touch::Event::Type::Continue, e.x, e.y }));
	}
	else if (e.type == Platform::Mouse::Event::Type::ButtonUp)
	{
		event(Platform::Touch::Event({ Platform::Touch::Event::Type::End, e.x, e.y }));
	}
}

void Scene::Scene::event(const Platform::Touch::Event& e)
{
	auto pos = glm::vec2(static_cast<float>(e.x), static_cast<float>(e.y));

	auto executeTouchedNodes = [this](auto type, auto pos) {
		for (auto node : mTouchedNodes)
		{
			if (node.expired())
				continue;

			node.lock()->touch(type, pos);
		}
	};

	if (e.type == Platform::Touch::Event::Type::Begin && mTouchedNodes.empty())
	{
		int mask = 0;

		for (auto node : getTouchableNodes(pos))
		{
			if ((mask & node->getTouchMask()) > 0)
				continue;

			mask |= node->getTouchMask();

			mTouchedNodes.push_back(node);
		}

		executeTouchedNodes(Node::Touch::Begin, pos);
	}
	else if (e.type == Platform::Touch::Event::Type::Continue && !mTouchedNodes.empty())
	{
		executeTouchedNodes(Node::Touch::Continue, pos);	
	}
	else if (e.type == Platform::Touch::Event::Type::End && !mTouchedNodes.empty())
	{
		executeTouchedNodes(Node::Touch::End, pos);
		mTouchedNodes.clear();
	}
}