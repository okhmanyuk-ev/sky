#include "common_actions.h"
#include <Console/device.h>

using namespace Shared;

// change position by direction

CommonActions::Action CommonActions::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed)
{
	return Shared::CommonActions::ExecuteInfinite([node, direction, speed] {
		auto dTime = Clock::ToSeconds(FRAME->getTimeDelta());
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

CommonActions::Action CommonActions::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration)
{
	return MakeParallel(Common::Actions::Parallel::Awaiting::Any,
		std::make_unique<Common::Actions::Wait>(duration),
		ChangePositionByDirection(node, direction, speed)
	);
}

// delayed

CommonActions::Action CommonActions::Delayed(float duration, Action action)
{
    return MakeSequence(
        std::make_unique<Common::Actions::Wait>(duration),
        std::move(action)
    );
}

// generic execute

CommonActions::Action CommonActions::Execute(std::function<void()> callback)
{
	return std::make_unique<Common::Actions::Generic>(Common::Actions::Generic::Type::One, callback);
}

CommonActions::Action CommonActions::ExecuteInfinite(std::function<void()> callback)
{
	return std::make_unique<Common::Actions::Generic>(Common::Actions::Generic::Type::Infinity, callback);
}

// log

CommonActions::Action CommonActions::Log(const std::string& text)
{
	return Execute([text] {
		LOG(text);
	});
}

// interploate

CommonActions::Action CommonActions::InterpolateValue(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(startValue, destValue, duration, easingFunction, [&value](float _value) {
		value = _value;
	});
}

CommonActions::Action CommonActions::InterpolateValue(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([destValue, duration, &value, easingFunction] {
		return InterpolateValue(value, destValue, duration, value, easingFunction);
	});
}

// color

CommonActions::Action CommonActions::ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setColor(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

// alpha

CommonActions::Action CommonActions::ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(start, dest, duration, easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

CommonActions::Action CommonActions::ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeAlpha(node, node->getAlpha(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::Hide(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

CommonActions::Action CommonActions::Show(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

// other

CommonActions::Action CommonActions::Shake(SceneTransform node, float radius, float duration)
{
	return MakeSequence(
		MakeParallel(Common::Actions::Parallel::Awaiting::Any,
			std::make_unique<Common::Actions::Wait>(duration),
			std::make_unique<Common::Actions::Repeat>([node, radius] {
				return Execute([node, radius] {
					node->setOrigin(glm::circularRand(radius));
				});
			})
		),
		Execute([node] {
			node->setOrigin({ 0.0f, 0.0f });
		})
	);
}

CommonActions::Action CommonActions::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		FRAME->addOne([node] {
			node->getParent()->detach(node);
		});
	});
}

CommonActions::Action CommonActions::ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalAnchor(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalAnchor(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalAnchor(node, node->getVerticalAnchor(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalAnchor(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalAnchor(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalPivot(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalPivot(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalPivot(node, node->getVerticalPivot(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPivot(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPivot(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalPosition(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalPosition(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalPosition(node, node->getVerticalPosition(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPosition(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPosition(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalSize(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalSize(node, node->getVerticalSize(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalSize(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalSize(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalStretch(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalStretch(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalStretch(node, node->getVerticalStretch(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalStretch(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalStretch(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setHorizontalScale(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalScale(glm::lerp(start, dest, value));
	});
}

CommonActions::Action CommonActions::ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalScale(node, node->getVerticalScale(), dest, duration, easingFunction);
	});
}

CommonActions::Action CommonActions::ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScale(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScale(node, start.y, dest.y, duration, easingFunction)
	);
}

CommonActions::Action CommonActions::ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}