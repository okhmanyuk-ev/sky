#include "actions.h"
#include <shared/scene_helpers.h> // TODO: we should not include shared from scene

using namespace Actions;

// change position by direction

Factory::UAction Factory::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed)
{
	return ExecuteInfinite([node, direction, speed] {
		auto dTime = Clock::ToSeconds(FRAME->getTimeDelta());
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

Factory::UAction Factory::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration)
{
	return Breakable(duration, ChangePositionByDirection(node, direction, speed));
}

// color

Factory::UAction Factory::ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec3& value) {
		node->setColor(value);
	});
}

Factory::UAction Factory::ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeColorRecursive(SceneNode node, const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec4& value) {
		Shared::SceneHelpers::RecursiveColorSet(node, value);
	});
}

// alpha

Factory::UAction Factory::ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

Factory::UAction Factory::ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAlpha(node, node->getAlpha(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::Hide(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

Factory::UAction Factory::Show(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

// other

Factory::UAction Factory::Shake(SceneTransform node, float radius, float duration)
{
	return MakeSequence(
		Breakable(duration, RepeatInfinite([node, radius] {
			return Execute([node, radius] {
				node->setOrigin(glm::circularRand(radius));
			});
		})),
		Execute([node] {
			node->setOrigin({ 0.0f, 0.0f });
		})
	);
}

Factory::UAction Factory::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		FRAME->addOne([node] {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
		});
	});
}

Factory::UAction Factory::ChangeRotation(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRotation(value);
	});
}

Factory::UAction Factory::ChangeRotation(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRotation(node, node->getRotation(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalAnchor(value);
	});
}

Factory::UAction Factory::ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalAnchor(value);
	});
}

Factory::UAction Factory::ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalAnchor(node, node->getVerticalAnchor(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalAnchor(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalAnchor(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPivot(value);
	});
}

Factory::UAction Factory::ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPivot(value);
	});
}

Factory::UAction Factory::ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPivot(node, node->getVerticalPivot(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPivot(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPivot(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPosition(value);
	});
}

Factory::UAction Factory::ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPosition(value);
	});
}

Factory::UAction Factory::ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPosition(node, node->getVerticalPosition(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPosition(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPosition(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalOrigin(value);
	});
}

Factory::UAction Factory::ChangeHorizontalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalOrigin(node, node->getHorizontalOrigin(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalOrigin(value);
	});
}

Factory::UAction Factory::ChangeVerticalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalOrigin(node, node->getVerticalOrigin(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeOrigin(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalOrigin(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalOrigin(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeOrigin(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeOrigin(node, node->getOrigin(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalSize(value);
	});
}

Factory::UAction Factory::ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(value);
	});
}

Factory::UAction Factory::ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalSize(node, node->getVerticalSize(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalSize(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalSize(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalStretch(value);
	});
}

Factory::UAction Factory::ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalStretch(value);
	});
}

Factory::UAction Factory::ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalStretch(node, node->getVerticalStretch(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalStretch(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalStretch(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalScale(value);
	});
}

Factory::UAction Factory::ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalScale(value);
	});
}

Factory::UAction Factory::ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalScale(node, node->getVerticalScale(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScale(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScale(node, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setPie(value);
	});
}

Factory::UAction Factory::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCirclePie(circle, circle->getPie(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setHorizontalScrollPosition(value);
	});
}

Factory::UAction Factory::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeHorizontalScrollPosition(scrollbox, scrollbox->getHorizontalScrollPosition(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setVerticalScrollPosition(value);
	});
}

Factory::UAction Factory::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeVerticalScrollPosition(scrollbox, scrollbox->getVerticalScrollPosition(), dest, duration, easingFunction);
	});
}

Factory::UAction Factory::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScrollPosition(scrollbox, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScrollPosition(scrollbox, start.y, dest.y, duration, easingFunction)
	);
}

Factory::UAction Factory::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeScrollPosition(scrollbox, scrollbox->getScrollPosition(), dest, duration, easingFunction);
	});
}

