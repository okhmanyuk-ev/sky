#include "actions.h"
#include <shared/scene_helpers.h> // TODO: we should not include shared from scene

using namespace Actions;

// change position by direction

std::unique_ptr<Action> Collection::ChangePositionByDirection(std::shared_ptr<Scene::Transform> node, const glm::vec2& direction, float speed)
{
	return ExecuteInfinite([node, direction, speed](auto delta) {
		auto dTime = sky::ToSeconds(delta);
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

std::unique_ptr<Action> Collection::ChangePositionByDirection(std::shared_ptr<Scene::Transform> node, const glm::vec2& direction, float speed, float duration)
{
	return Breakable(duration, ChangePositionByDirection(node, direction, speed));
}

// color

std::unique_ptr<Action> Collection::ChangeColor(std::shared_ptr<Scene::Color> node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec3& value) {
		node->setColor(value);
	});
}

std::unique_ptr<Action> Collection::ChangeColor(std::shared_ptr<Scene::Color> node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeColorRecursive(std::shared_ptr<Scene::Node> node, const glm::vec4& start,
	const glm::vec4& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec4& value) {
		Shared::SceneHelpers::RecursiveColorSet(node, value);
	});
}

// alpha

std::unique_ptr<Action> Collection::ChangeAlpha(std::shared_ptr<Scene::Color> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

std::unique_ptr<Action> Collection::ChangeAlpha(std::shared_ptr<Scene::Color> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAlpha(node, node->getAlpha(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::Hide(std::shared_ptr<Scene::Color> node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

std::unique_ptr<Action> Collection::Show(std::shared_ptr<Scene::Color> node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

std::unique_ptr<Action> Collection::HideRecursive(std::shared_ptr<Scene::Node> node, float duration,
	EasingFunction easingFunction)
{
	auto parallel = MakeParallel();
	std::function<void(std::shared_ptr<Scene::Node> node)> recursive_fill_func;
	recursive_fill_func = [&](std::shared_ptr<Scene::Node> node){
		for (auto child : node->getNodes())
		{
			recursive_fill_func(child);
		}

		auto color_node = std::dynamic_pointer_cast<Scene::Color>(node);

		if (!color_node)
			return;

		parallel->add(Hide(color_node, duration, easingFunction));
	};
	recursive_fill_func(node);
	return parallel;
}

// other

std::unique_ptr<Action> Collection::Shake(std::shared_ptr<Scene::Transform> node, float radius, float duration)
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

std::unique_ptr<Action> Collection::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		SCHEDULER->addOne([node] {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
		});
	});
}

std::unique_ptr<Action> Collection::ChangeRotation(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRotation(value);
	});
}

std::unique_ptr<Action> Collection::ChangeRotation(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRotation(node, node->getRotation(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalAnchor(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalAnchor(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalAnchor(node, node->getVerticalAnchor(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeAnchor(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalAnchor(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalAnchor(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeAnchor(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalPivot(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPivot(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalPivot(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPivot(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPivot(node, node->getVerticalPivot(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangePivot(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPivot(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPivot(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangePivot(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalPosition(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPosition(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalPosition(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalPosition(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPosition(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalPosition(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalPosition(node, node->getVerticalPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangePosition(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPosition(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPosition(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangePosition(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalOrigin(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalOrigin(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalOrigin(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalOrigin(node, node->getHorizontalOrigin(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalOrigin(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalOrigin(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalOrigin(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalOrigin(node, node->getVerticalOrigin(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeOrigin(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalOrigin(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalOrigin(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeOrigin(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeOrigin(node, node->getOrigin(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalSize(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalSize(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalSize(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalSize(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalSize(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalSize(node, node->getVerticalSize(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeSize(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalSize(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalSize(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeSize(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalStretch(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalStretch(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalStretch(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalStretch(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalStretch(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalStretch(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalStretch(node, node->getVerticalStretch(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeStretch(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalStretch(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalStretch(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeStretch(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalScale(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalScale(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalScale(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalScale(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalScale(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalScale(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalScale(node, node->getVerticalScale(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeScale(std::shared_ptr<Scene::Transform> node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScale(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScale(node, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeScale(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setPie(value);
	});
}

std::unique_ptr<Action> Collection::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCirclePie(circle, circle->getPie(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeCirclePiePivot(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setPiePivot(value);
	});
}

std::unique_ptr<Action> Collection::ChangeCirclePiePivot(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCirclePiePivot(circle, circle->getPiePivot(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setRadius(value);
	});
}

std::unique_ptr<Action> Collection::ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCircleRadius(circle, circle->getRadius(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setFill(value);
	});
}

std::unique_ptr<Action> Collection::ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCircleFill(circle, circle->getFill(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setHorizontalScrollPosition(value);
	});
}

std::unique_ptr<Action> Collection::ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeHorizontalScrollPosition(scrollbox, scrollbox->getHorizontalScrollPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [scrollbox](float value) {
		scrollbox->setVerticalScrollPosition(value);
	});
}

std::unique_ptr<Action> Collection::ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeVerticalScrollPosition(scrollbox, scrollbox->getVerticalScrollPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScrollPosition(scrollbox, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScrollPosition(scrollbox, start.y, dest.y, duration, easingFunction)
	);
}

std::unique_ptr<Action> Collection::ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([scrollbox, dest, duration, easingFunction] {
		return ChangeScrollPosition(scrollbox, scrollbox->getScrollPosition(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeRadialAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRadialAnchor(value);
	});
}

std::unique_ptr<Action> Collection::ChangeRadialAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRadialAnchor(node, node->getRadialAnchor(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeRadialPivot(std::shared_ptr<Scene::Transform> node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRadialPivot(value);
	});
}

std::unique_ptr<Action> Collection::ChangeRadialPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRadialPivot(node, node->getRadialPivot(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeBlurIntensity(std::shared_ptr<Scene::BlurredGlass> blurred_glass, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [blurred_glass](float value) {
		blurred_glass->setBlurIntensity(value);
	});
}

std::unique_ptr<Action> Collection::ChangeBlurIntensity(std::shared_ptr<Scene::BlurredGlass> blurred_glass, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([blurred_glass, dest, duration, easingFunction] {
		return ChangeBlurIntensity(blurred_glass, blurred_glass->getBlurIntensity(), dest, duration, easingFunction);
	});
}

std::unique_ptr<Action> Collection::ChangeGrayscaleIntensity(std::shared_ptr<Scene::GrayscaledGlass> grayscaled_glass, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [grayscaled_glass](float value) {
		grayscaled_glass->setGrayscaleIntensity(value);
	});
}

std::unique_ptr<Action> Collection::ChangeGrayscaleIntensity(std::shared_ptr<Scene::GrayscaledGlass> grayscaled_glass, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([grayscaled_glass, dest, duration, easingFunction] {
		return ChangeGrayscaleIntensity(grayscaled_glass, grayscaled_glass->getGrayscaleIntensity(), dest, duration, easingFunction);
	});
}
