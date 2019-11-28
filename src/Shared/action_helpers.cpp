#include "action_helpers.h"
#include <Console/device.h>

using namespace Shared;

// change position by direction

ActionHelpers::Action ActionHelpers::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed)
{
	return ExecuteInfinite([node, direction, speed] {
		auto dTime = Clock::ToSeconds(FRAME->getTimeDelta());
		node->setPosition(node->getPosition() + direction * dTime * speed);
	});
}

ActionHelpers::Action ActionHelpers::ChangePositionByDirection(SceneTransform node, const glm::vec2& direction, float speed, float duration)
{
	return MakeParallel(Common::Actions::Parallel::Awaiting::Any,
		Wait(duration),
		ChangePositionByDirection(node, direction, speed)
	);
}

// wait

ActionHelpers::Action ActionHelpers::Wait(float duration)
{
	return std::make_unique<Common::Actions::Wait>(Clock::FromSeconds(duration));
}

ActionHelpers::Action ActionHelpers::Wait(std::function<bool()> while_callback)
{
	return std::make_unique<Common::Actions::Generic>([while_callback] {
		if (while_callback())
			return Common::Actions::Action::Status::Continue;

		return Common::Actions::Action::Status::Finished;
	});
}

// delayed

ActionHelpers::Action ActionHelpers::Delayed(float duration, Action action)
{
    return MakeSequence(
        Wait(duration),
        std::move(action)
    );
}

ActionHelpers::Action ActionHelpers::Delayed(std::function<bool()> while_callback, Action action)
{
	return MakeSequence(
		Wait(while_callback),
		std::move(action)
	);
}

// generic execute

ActionHelpers::Action ActionHelpers::Execute(std::function<void()> callback)
{
	return std::make_unique<Common::Actions::Generic>(Common::Actions::Generic::Type::One, callback);
}

ActionHelpers::Action ActionHelpers::ExecuteInfinite(std::function<void()> callback)
{
	return std::make_unique<Common::Actions::Generic>(Common::Actions::Generic::Type::Infinity, callback);
}

// log

ActionHelpers::Action ActionHelpers::Log(const std::string& text)
{
	return Execute([text] {
		LOG(text);
	});
}

// interploate

ActionHelpers::Action ActionHelpers::InterpolateValue(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(startValue, destValue, Clock::FromSeconds(duration), easingFunction, [&value](float _value) {
		value = _value;
	});
}

ActionHelpers::Action ActionHelpers::InterpolateValue(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([destValue, duration, &value, easingFunction] {
		return InterpolateValue(value, destValue, duration, value, easingFunction);
	});
}

// color

ActionHelpers::Action ActionHelpers::ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setColor(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

// alpha

ActionHelpers::Action ActionHelpers::ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(start, dest, Clock::FromSeconds(duration), easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeAlpha(node, node->getAlpha(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::Hide(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 0.0f, duration, easingFunction);
}

ActionHelpers::Action ActionHelpers::Show(SceneColor node, float duration, EasingFunction easingFunction)
{
	return ChangeAlpha(node, 1.0f, duration, easingFunction);
}

// other

ActionHelpers::Action ActionHelpers::Shake(SceneTransform node, float radius, float duration)
{
	return MakeSequence(
		MakeParallel(Common::Actions::Parallel::Awaiting::Any,
			Wait(duration),
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

ActionHelpers::Action ActionHelpers::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		FRAME->addOne([node] {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
		});
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalAnchor(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalAnchor(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalAnchor(node, node->getVerticalAnchor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeAnchor(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalAnchor(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalAnchor(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangeAnchor(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalPivot(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalPivot(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalPivot(node, node->getVerticalPivot(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangePivot(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPivot(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPivot(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangePivot(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalPosition(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalPosition(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalPosition(node, node->getVerticalPosition(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangePosition(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalPosition(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalPosition(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangePosition(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalSize(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalSize(node, node->getVerticalSize(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeSize(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalSize(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalSize(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangeSize(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalStretch(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalStretch(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalStretch(node, node->getVerticalStretch(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeStretch(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalStretch(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalStretch(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangeStretch(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setHorizontalScale(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Interpolate>(0.0f, 1.0f, Clock::FromSeconds(duration), easingFunction, [node, start, dest](float value) {
		node->setVerticalScale(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeVerticalScale(node, node->getVerticalScale(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeScale(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalScale(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalScale(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangeScale(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return std::make_unique<Common::Actions::Insert>([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}