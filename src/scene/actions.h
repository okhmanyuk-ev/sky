#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/circle.h>
#include <scene/scrollbox.h>
#include <scene/glass.h>
#include <common/actions.h>
#include <common/easing.h>

namespace Actions::Collection
{
	std::unique_ptr<Action> ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed);
	std::unique_ptr<Action> ChangePositionByDirection(std::shared_ptr<Scene::Transform> node,
		const glm::vec2& direction, float speed, float duration);

	std::unique_ptr<Action> ChangeColor(std::shared_ptr<Scene::Color> node, const glm::vec3& start,
		const glm::vec3& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeColor(std::shared_ptr<Scene::Color> node, const glm::vec3& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeColorRecursive(std::shared_ptr<Scene::Node> node, const glm::vec4& start,
		const glm::vec4& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeAlpha(std::shared_ptr<Scene::Color> node, float start, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeAlpha(std::shared_ptr<Scene::Color> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> Hide(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> Show(std::shared_ptr<Scene::Color> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> HideRecursive(std::shared_ptr<Scene::Node> node, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> Shake(std::shared_ptr<Scene::Transform> node, float radius, float duration);
	std::unique_ptr<Action> Kill(std::shared_ptr<Scene::Node> node);

	std::unique_ptr<Action> ChangeRotation(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeRotation(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeAnchor(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeAnchor(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalPivot(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalPivot(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangePivot(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangePivot(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalPosition(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalPosition(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);
	
	std::unique_ptr<Action> ChangeVerticalPosition(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalPosition(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangePosition(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangePosition(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalOrigin(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalOrigin(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalOrigin(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalOrigin(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeOrigin(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeOrigin(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalSize(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalSize(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);
	
	std::unique_ptr<Action> ChangeVerticalSize(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalSize(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeSize(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeSize(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalStretch(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalStretch(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalStretch(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalStretch(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeStretch(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeStretch(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalScale(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalScale(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalScale(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalScale(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeScale(std::shared_ptr<Scene::Transform> node, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeScale(std::shared_ptr<Scene::Transform> node, const glm::vec2& dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeCirclePiePivot(std::shared_ptr<Scene::Circle> circle, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeCirclePiePivot(std::shared_ptr<Scene::Circle> circle, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeCircleRadius(std::shared_ptr<Scene::Circle> circle, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeCircleFill(std::shared_ptr<Scene::Circle> circle, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start,
		float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start,
		float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& start,
		const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeRadialAnchor(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeRadialAnchor(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeRadialPivot(std::shared_ptr<Scene::Transform> node, float start, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeRadialPivot(std::shared_ptr<Scene::Transform> node, float dest, float duration,
		EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeBlurIntensity(std::shared_ptr<Scene::BlurredGlass> blurred_glass, float start,
		float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeBlurIntensity(std::shared_ptr<Scene::BlurredGlass> blurred_glass, float dest,
		float duration, EasingFunction easingFunction = Easing::Linear);

	std::unique_ptr<Action> ChangeGrayscaleIntensity(std::shared_ptr<Scene::GrayscaledGlass> grayscaled_glass,
		float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	std::unique_ptr<Action> ChangeGrayscaleIntensity(std::shared_ptr<Scene::GrayscaledGlass> grayscaled_glass,
		float dest, float duration, EasingFunction easingFunction = Easing::Linear);
}