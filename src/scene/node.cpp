#include "node.h"
#include <scene/scene.h>

#include <algorithm>
#include <cassert>

using namespace Scene;

bool Node::Bounds::isIntersect(const Bounds& other) const
{
	auto max = pos + size;
	auto other_max = other.pos + other.size;
	return max.x > other.pos.x && pos.x < other_max.x && max.y > other.pos.y && pos.y < other_max.y;
}

Node::Node()
{
}

Node::~Node()
{
	// we should setup parent to nullptr for every child,
	// because they can be stored locally somewhere

	for (auto node : mNodes)
	{
		node->mParent = nullptr;
	}
}

void Node::attach(std::shared_ptr<Node> node, AttachDirection attach_direction)
{
	assert(node->mParent == nullptr);
	if (attach_direction == AttachDirection::Back)
	{
		mNodes.push_back(node);
	}
	else
	{
		mNodes.push_front(node);
	}
	node->mParent = this;
}

void Node::detach(std::shared_ptr<Node> node)
{
	assert(node->mParent == this);
	mNodes.remove(node);
	node->mParent = nullptr;
}

void Node::clear()
{
	while (!mNodes.empty())
	{
		detach(mNodes.front());
	}
}

glm::vec2 Node::project(const glm::vec2& value) const
{
	if (!mTransformReady)
		throw std::runtime_error("transform isn't ready");

	auto scene = getScene();

	if (scene == nullptr)
		throw std::runtime_error("scene is null");

	auto vp = scene->getViewport();
	auto scaled_size = vp.size / PLATFORM->getScale();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, scaled_size.x, scaled_size.y, 0.0f, -1.0f, 1.0f);
	glm::vec4 viewport = { 0.0f, vp.size.y, vp.size.x, -vp.size.y };
	glm::vec3 result = glm::project(original, getTransform(), projection, viewport);

	return { result.x, result.y };
}

glm::vec2 Node::unproject(const glm::vec2& value) const
{
	if (!mTransformReady)
		throw std::runtime_error("transform isn't ready");

	auto scene = getScene();

	if (scene == nullptr)
		throw std::runtime_error("scene is null");

	auto vp = scene->getViewport();
	auto scaled_size = vp.size / PLATFORM->getScale();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, scaled_size.x, scaled_size.y, 0.0f, -1.0f, 1.0f);
	glm::vec4 viewport = { 0.0f, vp.size.y, vp.size.x, -vp.size.y };
	glm::vec3 result = glm::unProject(original, getTransform(), projection, viewport);

	return { result.x, result.y };
}

Node::Bounds Node::getGlobalBounds() const
{
	auto tl = project({ 0.0f, 0.0f });
	auto tr = project({ getAbsoluteWidth(), 0.0f });
	auto bl = project({ 0.0f, getAbsoluteHeight() });
	auto br = project({ getAbsoluteWidth(), getAbsoluteHeight() });

	auto min = glm::min(glm::min(tl, tr), glm::min(bl, br));
	auto max = glm::max(glm::max(tl, tr), glm::max(bl, br));

	return Bounds{
		.pos = min,
		.size = max - min
	};
}

::Scene::Scene* Node::getScene() const
{
	if (!hasParent())
		return nullptr;

	return getParent()->getScene();
}

bool Node::hitTest(const glm::vec2& value) const
{
	return
		value.x >= 0.0f &&
		value.y >= 0.0f &&
		value.x <= getAbsoluteWidth() &&
		value.y <= getAbsoluteHeight();
}

bool Node::interactTest(const glm::vec2& value) const
{
	return true;
}

void Node::updateTransform()
{
	auto parent_size = hasParent() ? getParent()->getAbsoluteSize() : (getScene()->getViewport().size / PLATFORM->getScale());

	mTransform = hasParent() ? getParent()->getTransform() : glm::mat4(1.0f);
	mTransform = glm::translate(mTransform, { getAnchor() * parent_size, 0.0f });
	mTransform = glm::translate(mTransform, { getPosition(), 0.0f });
	mTransform = glm::rotate(mTransform, getRotation(), { 0.0f, 0.0f, 1.0f });
	mTransform = glm::rotate(mTransform, getRadialAnchor() * glm::pi<float>() * 2.0f, { 0.0f, 0.0f, 1.0f });
	mTransform = glm::rotate(mTransform, -getRadialPivot() * glm::pi<float>() * 2.0f, { 0.0f, 0.0f, 1.0f });
	mTransform = glm::scale(mTransform, { getScale(), 1.0f });
	mTransform = glm::translate(mTransform, { -getPivot() * getAbsoluteSize(), 0.0f });
	mTransform = glm::translate(mTransform, { -getOrigin(), 0.0f });

	mTransformReady = true;
}

void Node::updateAbsoluteSize()
{
	auto parent_size = hasParent() ? getParent()->getAbsoluteSize() : getScene()->getViewport().size;

	mAbsoluteSize = getSize();
	mAbsoluteSize -= getMargin();
	mAbsoluteSize += getStretch() * parent_size;
}

void Node::updateAbsoluteScale()
{
	auto parent_scale = hasParent() ? getParent()->getAbsoluteScale() : glm::vec2{ 1.0f, 1.0f };
	mAbsoluteScale = getScale() * parent_scale;
}

void Node::enterUpdate()
{
}

void Node::update(sky::Duration dTime)
{
	mActions.update(dTime);
	updateAbsoluteSize();
	updateAbsoluteScale();
	updateTransform();
}

void Node::leaveUpdate()
{
}

void Node::enterDraw()
{
}

void Node::draw()
{
}

void Node::leaveDraw()
{
}

void Node::touch(Touch type, const glm::vec2& pos)
{
	if (type == Touch::Begin)
		mTouching = true;
	else if (type == Touch::End)
		mTouching = false;
}

void Node::scroll(float x, float y)
{
}
