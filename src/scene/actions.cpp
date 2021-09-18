#include "actions.h"
#include <shared/scene_helpers.h> // TODO: we should not include shared from scene

using namespace Actions;

// change position by direction

Collection::UAction Collection::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed)
{
	return ExecuteInfinite([node, direction, speed](auto delta) {
		auto dTime = Clock::ToSeconds(delta);
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

Collection::UAction Collection::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration)
{
	return Breakable(duration, ChangePositionByDirection(node, direction, speed));
}

// color

Collection::UAction Collection::ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec3& value) {
		node->setColor(value);
	});
}

Collection::UAction Collection::ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeColorRecursive(SceneNode node, const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec4& value) {
		Shared::SceneHelpers::RecursiveColorSet(node, value);
	});
}

// alpha

Collection::UAction Collection::ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

Collection::UAction Collection::ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAlpha(node, node->getAlpha(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::Hide(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

Collection::UAction Collection::Show(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

// other

Collection::UAction Collection::Shake(SceneTransform node, float radius, float duration)
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

Collection::UAction Collection::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		FRAME->addOne([node] {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
		});
	});
}

Collection::UAction Collection::ChangeRotation(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRotation(value);
	});
}

Collection::UAction Collection::ChangeRotation(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRotation(node, node->getRotation(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalAnchor(value);
	});
}

Collection::UAction Collection::ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalAnchor(value);
	});
}

Collection::UAction Collection::ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalAnchor(node, node->getVerticalAnchor(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalAnchor(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalAnchor(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPivot(value);
	});
}

Collection::UAction Collection::ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPivot(value);
	});
}

Collection::UAction Collection::ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPivot(node, node->getVerticalPivot(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPivot(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPivot(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPosition(value);
	});
}

Collection::UAction Collection::ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPosition(value);
	});
}

Collection::UAction Collection::ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPosition(node, node->getVerticalPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPosition(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPosition(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalOrigin(value);
	});
}

Collection::UAction Collection::ChangeHorizontalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalOrigin(node, node->getHorizontalOrigin(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalOrigin(value);
	});
}

Collection::UAction Collection::ChangeVerticalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalOrigin(node, node->getVerticalOrigin(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeOrigin(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalOrigin(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalOrigin(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeOrigin(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeOrigin(node, node->getOrigin(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalSize(value);
	});
}

Collection::UAction Collection::ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(value);
	});
}

Collection::UAction Collection::ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalSize(node, node->getVerticalSize(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalSize(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalSize(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalStretch(value);
	});
}

Collection::UAction Collection::ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalStretch(value);
	});
}

Collection::UAction Collection::ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalStretch(node, node->getVerticalStretch(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalStretch(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalStretch(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalScale(value);
	});
}

Collection::UAction Collection::ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalScale(value);
	});
}

Collection::UAction Collection::ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalScale(node, node->getVerticalScale(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScale(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScale(node, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setPie(value);
	});
}

Collection::UAction Collection::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCirclePie(circle, circle->getPie(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setRadius(value);
	});
}

Collection::UAction Collection::ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCircleRadius(circle, circle->getRadius(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setFill(value);
	});
}

Collection::UAction Collection::ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCircleFill(circle, circle->getFill(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setHorizontalScrollPosition(value);
	});
}

Collection::UAction Collection::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeHorizontalScrollPosition(scrollbox, scrollbox->getHorizontalScrollPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setVerticalScrollPosition(value);
	});
}

Collection::UAction Collection::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeVerticalScrollPosition(scrollbox, scrollbox->getVerticalScrollPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScrollPosition(scrollbox, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScrollPosition(scrollbox, start.y, dest.y, duration, easingFunction)
	);
}

Collection::UAction Collection::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeScrollPosition(scrollbox, scrollbox->getScrollPosition(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeRadialAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRadialAnchor(value);
	});
}

Collection::UAction Collection::ChangeRadialAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRadialAnchor(node, node->getRadialAnchor(), dest, duration, easingFunction);
	});
}

Collection::UAction Collection::ChangeBlurIntensity(std::shared_ptr<Scene::Blur> blur, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [blur](float value) {
		blur->setBlurIntensity(value);
	});
}

Collection::UAction Collection::ChangeBlurIntensity(std::shared_ptr<Scene::Blur> blur, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([blur, dest, duration, easingFunction] {
		return ChangeBlurIntensity(blur, blur->getBlurIntensity(), dest, duration, easingFunction);
	});
}
