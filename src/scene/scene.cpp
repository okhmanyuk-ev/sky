#include "scene.h"

Scene::Scene::Scene()
{
	mRoot->setStretch(1.0f);
	mRoot->setScene(this);
}

Scene::Scene::~Scene()
{
	//
}

void Scene::Scene::recursiveNodeUpdateTransform(std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	node->updateTransform();

	for (auto _node : node->getNodes())
		recursiveNodeUpdateTransform(_node);
}

void Scene::Scene::recursiveNodeUpdate(std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	node->update();

	for (auto _node : node->getNodes())
		recursiveNodeUpdate(_node);
}

void Scene::Scene::recursiveNodeDraw(std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	if (!node->isVisible())
		return;

	if (!node->isTransformReady())
		return;

	node->enterDraw();
	node->draw();

	for (auto _node : node->getNodes())
		recursiveNodeDraw(_node);

	node->leaveDraw();
}

bool Scene::Scene::interactTest(const glm::vec2& pos)
{
	if (!mInteractTestCallback)
		return true;

	return mInteractTestCallback(pos);
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getTouchableNodes(std::shared_ptr<Node> node, const glm::vec2& pos)
{
	if (!node->isEnabled())
		return { };

	if (!node->isInteractions())
		return { };

	if (!node->isTransformReady())
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

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getNodes(std::shared_ptr<Node> node, const glm::vec2& pos)
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
	mViewport = Renderer::Viewport(mRenderTarget);

	recursiveNodeUpdate(mRoot);
	recursiveNodeUpdateTransform(mRoot);

	GRAPHICS->begin();
	GRAPHICS->pushRenderTarget(mRenderTarget);
	GRAPHICS->pushViewport(mRenderTarget);
	GRAPHICS->pushOrthoMatrix(mRenderTarget);
	recursiveNodeDraw(mRoot);
	GRAPHICS->pop(3);
	GRAPHICS->end();
}

size_t Scene::Scene::getNodesCount(std::shared_ptr<Node> node) const
{
	if (node == nullptr)
		return getNodesCount(mRoot);
	
	size_t result = 1;

	for (auto _node : node->getNodes())
	{
		result += getNodesCount(_node);
	}

	return result;
}

void Scene::Scene::event(const Platform::Input::Mouse::Event& e)
{
	if (e.type == Platform::Input::Mouse::Event::Type::ButtonDown)
	{
		event(Platform::Input::Touch::Event({ Platform::Input::Touch::Event::Type::Begin, e.x, e.y }));
	}
	else if (e.type == Platform::Input::Mouse::Event::Type::Move)
	{
		event(Platform::Input::Touch::Event({ Platform::Input::Touch::Event::Type::Continue, e.x, e.y }));
	}
	else if (e.type == Platform::Input::Mouse::Event::Type::ButtonUp)
	{
		event(Platform::Input::Touch::Event({ Platform::Input::Touch::Event::Type::End, e.x, e.y }));
	}
}

void Scene::Scene::event(const Platform::Input::Touch::Event& e)
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

	if (e.type == Platform::Input::Touch::Event::Type::Begin && mTouchedNodes.empty() && interactTest(pos))
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
	else if (e.type == Platform::Input::Touch::Event::Type::Continue && !mTouchedNodes.empty())
	{
		executeTouchedNodes(Node::Touch::Continue, pos);	
	}
	else if (e.type == Platform::Input::Touch::Event::Type::End && !mTouchedNodes.empty())
	{
		executeTouchedNodes(Node::Touch::End, pos);
		mTouchedNodes.clear();
	}
}