#include "scene.h"

Scene::Scene::Scene()
{
	mRoot->setStretch(1.0f);
	mRoot->setScene(this);

	mTimestepFixer.setTimestep(sky::FromSeconds(1.0f / 120.0f));
	mTimestepFixer.setForceTimeCompletion(true);
	mTimestepFixer.setSkipLongFrames(true);
}

Scene::Scene::~Scene()
{
}

void Scene::Scene::recursiveNodeUpdate(Node& node, sky::Duration delta)
{
	if (!node.isEnabled())
		return;

	node.enterUpdate();
	node.update(delta);

	for (auto _node : node.getNodes())
		recursiveNodeUpdate(*_node, delta);

	node.leaveUpdate();
}

void Scene::Scene::recursiveNodeDraw(Node& node)
{
	if (!node.isEnabled())
		return;

	if (!node.isVisible())
		return;

	if (!node.isTransformReady())
		return;

	node.enterDraw();

	const auto& batch_group = node.getBatchGroup();

	if (mBatchGroupsEnabled && batch_group.has_value())
	{
		drawBatchGroup(batch_group.value());
	}
	else
	{
		node.draw();
	}

	for (auto _node : node.getNodes())
		recursiveNodeDraw(*_node);

	node.leaveDraw();
}

void Scene::Scene::drawBatchGroup(const std::string& name)
{
	if (!mBatchGroups.contains(name))
		return;

	for (auto node : mBatchGroups.at(name))
	{
		if (node.expired())
			continue;

		node.lock()->draw();
	}

	mBatchGroups.erase(name);
}

bool Scene::Scene::interactTest(const glm::vec2& pos)
{
	if (!mInteractTestCallback)
		return true;

	return mInteractTestCallback(pos);
}

std::list<std::weak_ptr<Scene::Node>> Scene::Scene::getTouchedNodes(const glm::vec2& pos) const
{
	std::list<std::weak_ptr<Node>> result;
	int mask = 0;

	for (auto node : getTouchableNodes(pos))
	{
		if ((mask & node->getTouchMask()) > 0)
			continue;

		mask |= node->getTouchMask();

		result.push_back(node);
	}

	return result;
}

void Scene::Scene::MakeBatchLists(BatchGroups& batchGroups, std::shared_ptr<Node> node)
{
	if (!node->isEnabled())
		return;

	if (!node->isVisible())
		return;

	if (!node->isTransformReady())
		return;

	const auto& batch_group = node->getBatchGroup();

	if (batch_group.has_value())
		batchGroups[batch_group.value()].push_back(node);

	for (auto _node : node->getNodes())
		MakeBatchLists(batchGroups, _node);
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getTouchableNodes(std::shared_ptr<Node> node, const glm::vec2& pos) const
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

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getTouchableNodes(const glm::vec2& pos) const
{
	return getTouchableNodes(mRoot, pos);
}

std::list<std::shared_ptr<Scene::Node>> Scene::Scene::getNodes(std::shared_ptr<Node> node, const glm::vec2& pos)
{
	if (!node->isEnabled())
		return { };

	if (!node->isTransformReady())
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
	if (mRenderTarget)
	{
		mViewport.size.x = static_cast<float>(mRenderTarget->getWidth());
		mViewport.size.y = static_cast<float>(mRenderTarget->getHeight());
	}
	else
	{
		mViewport.size.x = static_cast<float>(PLATFORM->getWidth());
		mViewport.size.y = static_cast<float>(PLATFORM->getHeight());
		mViewport.size /= PLATFORM->getScale();
	}

	mTimestepFixer.execute([&](auto delta) {
		recursiveNodeUpdate(*mRoot, delta);
	});

	if (mScreenAdaption.has_value())
	{
		auto scale = mViewport.size / mScreenAdaption.value();
		auto min_scale = glm::min(scale.x, scale.y);
		mRoot->setScale(min_scale);
		mRoot->setStretch(1.0f / min_scale);
	}

	if (mBatchGroupsEnabled)
	{
		mBatchGroups.clear();
		MakeBatchLists(mBatchGroups, mRoot);
	}

	GRAPHICS->begin();
	GRAPHICS->pushRenderTarget(mRenderTarget);
	GRAPHICS->pushOrthoMatrix(mRenderTarget);
	recursiveNodeDraw(*mRoot);
	GRAPHICS->pop(2);
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

void Scene::Scene::onEvent(const Platform::Input::Mouse::ButtonEvent& e)
{
	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Pressed && e.button == Platform::Input::Mouse::Button::Left)
	{
		onEvent(Platform::Input::Touch::Event({
			.type = Platform::Input::Touch::Event::Type::Begin,
			.pos = e.pos,
		}));
	}
	else if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Released && e.button == Platform::Input::Mouse::Button::Left)
	{
		onEvent(Platform::Input::Touch::Event({
			.type = Platform::Input::Touch::Event::Type::End,
			.pos = e.pos
		}));
	}
}

void Scene::Scene::onEvent(const Platform::Input::Mouse::MoveEvent& e)
{
	onEvent(Platform::Input::Touch::Event({
		.type = Platform::Input::Touch::Event::Type::Continue,
		.pos = e.pos
	}));
}

void Scene::Scene::onEvent(const Platform::Input::Mouse::ScrollEvent& e)
{
	if (!interactTest(e.pos))
		return;

	auto nodes = getTouchableNodes(e.pos);

	for (auto node : nodes)
	{
		node->scroll(e.scroll.x, e.scroll.y);
	}
}

void Scene::Scene::onEvent(const Platform::Input::Touch::Event& e)
{
	auto pos = glm::vec2(e.pos);

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
		mTouchedNodes = getTouchedNodes(pos);
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
