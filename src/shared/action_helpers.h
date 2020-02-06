#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Common/actions.h>
#include <Common/easing.h>

namespace Shared::ActionHelpers
{
	using EasingFunction = Common::Actions::Interpolate::EasingFunction;
    using Action = std::unique_ptr<Common::Actions::Action>;
	using SceneTransform = std::shared_ptr<Scene::Transform>;
	using SceneColor = std::shared_ptr<Scene::Color>;

	Action Insert(std::function<Action()> action);
	Action RepeatInfinite(std::function<Action()> action);

	Action ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed);
	Action ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration);

	Action Wait(float duration);
	Action Wait(std::function<bool()> while_callback);
    
	Action Delayed(float duration, Action action);
	Action Delayed(std::function<bool()> while_callback, Action action);

	Action Execute(std::function<void()> callback);
	Action ExecuteInfinite(std::function<void()> callback);

	Action Log(const std::string& text);

	Action Interpolate(float startValue, float destValue, float duration, EasingFunction easingFunction, std::function<void(float)> callback);
	Action Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction = Common::Easing::Linear);
	Action Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action Hide(SceneColor node, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action Show(SceneColor node, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action Shake(SceneTransform node, float radius, float duration);
	Action Kill(std::shared_ptr<Scene::Node> node);

	Action ChangeRotation(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeRotation(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	
	Action ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	
	Action ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);

	Action ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
	Action ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction = Common::Easing::Linear);
}

namespace Shared::ActionHelpers
{
	template<class...Args> std::unique_ptr<Common::Actions::Sequence> MakeSequence(Args&&...args)
	{
		auto seq = std::make_unique<Common::Actions::Sequence>();
		(seq->add(std::forward<Args>(args)), ...);
		return seq;
	}

	template<class...Args> std::unique_ptr<Common::Actions::Parallel> MakeParallel(Common::Actions::Parallel::Awaiting awaitingType, Args&&...args)
	{
		auto parallel = std::make_unique<Common::Actions::Parallel>(awaitingType);
		(parallel->add(std::forward<Args>(args)), ...);
		return parallel;
	}

	template<class...Args> std::unique_ptr<Common::Actions::Parallel> MakeParallel(Args&&...args)
	{
		return MakeParallel(Common::Actions::Parallel::Awaiting::All, std::forward<Args>(args)...);
	}
}