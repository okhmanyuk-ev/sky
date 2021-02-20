#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/circle.h>
#include <scene/scrollbox.h>
#include <common/actions.h>
#include <common/easing.h>

namespace Actions::Factory
{
    using SceneTransform = std::shared_ptr<Scene::Transform>;
	using SceneColor = std::shared_ptr<Scene::Color>;
	using SceneNode = std::shared_ptr<Scene::Node>;

	UAction ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed);
	UAction ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration);

	UAction ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeColorRecursive(SceneNode node, const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction Hide(SceneColor node, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction Show(SceneColor node, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction Shake(SceneTransform node, float radius, float duration);
	UAction Kill(std::shared_ptr<Scene::Node> node);

	UAction ChangeRotation(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeRotation(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	
	UAction ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeOrigin(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeOrigin(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	
	UAction ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeHorizontalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeVerticalScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, float dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeScrollPosition(std::shared_ptr<Scene::Scrollbox> scrollbox, const glm::vec2& dest, float duration, EasingFunction easingFunction = Easing::Linear);

	UAction ChangeRadialAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
	UAction ChangeRadialAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Easing::Linear);
}