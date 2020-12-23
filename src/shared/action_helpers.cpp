#include "action_helpers.h"
#include <console/device.h>
#include <shared/scene_helpers.h>

using namespace Shared;

// insert

ActionHelpers::Action ActionHelpers::Insert(std::function<Action()> action)
{
	return std::make_unique<Actions::Repeat>([action]() -> Actions::Repeat::Result {
		return { Actions::Action::Status::Finished, action() };
	});
}

ActionHelpers::Action ActionHelpers::RepeatInfinite(std::function<Action()> action)
{
	return std::make_unique<Actions::Repeat>([action]() -> Actions::Repeat::Result {
		return { Actions::Action::Status::Continue, action() };
	});
}

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
	return Breakable(duration, ChangePositionByDirection(node, direction, speed));
}

// wait

ActionHelpers::Action ActionHelpers::Wait(float duration)
{
	return Wait([duration]() mutable {
		duration -= Clock::ToSeconds(FRAME->getTimeDelta());
		return duration > 0.0f;
	});
}

ActionHelpers::Action ActionHelpers::Wait(std::function<bool()> while_callback)
{
	return std::make_unique<Actions::Generic>([while_callback] {
		if (while_callback())
			return Actions::Action::Status::Continue;

		return Actions::Action::Status::Finished;
	});
}

ActionHelpers::Action ActionHelpers::WaitOneFrame()
{
	return Execute(nullptr);
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

// breakable

ActionHelpers::Action ActionHelpers::Breakable(float duration, Action action)
{
	return MakeParallel(Actions::Parallel::Awaiting::Any,
		Wait(duration),
		std::move(action)
	);
}

ActionHelpers::Action ActionHelpers::Breakable(std::function<bool()> while_callback, Action action)
{
	return MakeParallel(Actions::Parallel::Awaiting::Any,
		Wait(while_callback),
		std::move(action)
	);
}

// pausable

ActionHelpers::Action ActionHelpers::Pausable(std::function<bool()> run_callback, ActionHelpers::Action action)
{
	auto player = std::make_shared<Actions::GenericActionsPlayer<Actions::Parallel>>();
	player->add(std::move(action));

	return std::make_unique<Actions::Generic>([run_callback, player]() {
		if (!run_callback())
			return Actions::Action::Status::Continue;

		player->update();

		if (player->hasActions())
			return Actions::Action::Status::Continue;

		return Actions::Action::Status::Finished;
	});
}

// generic execute

ActionHelpers::Action ActionHelpers::Execute(std::function<void()> callback)
{
	return std::make_unique<Actions::Generic>(Actions::Generic::Type::One, callback);
}

ActionHelpers::Action ActionHelpers::ExecuteInfinite(std::function<void()> callback)
{
	return std::make_unique<Actions::Generic>(Actions::Generic::Type::Infinity, callback);
}

// log

ActionHelpers::Action ActionHelpers::Log(const std::string& text)
{
	return Execute([text] {
		LOG(text);
	});
}

// interploate

ActionHelpers::Action ActionHelpers::Interpolate(float start, float dest, float duration, EasingFunction easingFunction, std::function<void(float)> callback)
{
	return std::make_unique<Actions::Interpolate>(start, dest, Clock::FromSeconds(duration), easingFunction, callback);
}

ActionHelpers::Action ActionHelpers::Interpolate(const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec2&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec3&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction, std::function<void(const glm::vec4&)> callback)
{
	return Interpolate(0.0f, 1.0f, duration, easingFunction, [callback, start, dest](float value) {
		callback(glm::lerp(start, dest, value));
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(float startValue, float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](float _value) {
		value = _value;
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(float destValue, float duration, float& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Interpolate(value, destValue, duration, value, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(const glm::vec3& startValue, const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Interpolate(startValue, destValue, duration, easingFunction, [&value](const glm::vec3& _value) {
		value = _value;
	});
}

ActionHelpers::Action ActionHelpers::Interpolate(const glm::vec3& destValue, float duration, glm::vec3& value, EasingFunction easingFunction)
{
	return Insert([destValue, duration, &value, easingFunction] {
		return Interpolate(value, destValue, duration, value, easingFunction);
	});
}

// color

ActionHelpers::Action ActionHelpers::ChangeColor(SceneColor node, const glm::vec3& start, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec3& value) {
		node->setColor(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeColor(SceneColor node, const glm::vec3& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeColor(node, node->getColor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeColorRecursive(SceneNode node, const glm::vec4& start, const glm::vec4& dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](const glm::vec4& value) {
		SceneHelpers::RecursiveColorSet(node, value);
	});
}

// alpha

ActionHelpers::Action ActionHelpers::ChangeAlpha(SceneColor node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setAlpha(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeAlpha(SceneColor node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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

ActionHelpers::Action ActionHelpers::Kill(std::shared_ptr<Scene::Node> node)
{
	return Execute([node] {
		FRAME->addOne([node] {
			if (auto parent = node->getParent(); parent != nullptr)
				parent->detach(node);
		});
	});
}

ActionHelpers::Action ActionHelpers::ChangeRotation(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setRotation(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeRotation(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeRotation(node, node->getRotation(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalAnchor(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalAnchor(node, node->getHorizontalAnchor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalAnchor(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalAnchor(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalAnchor(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangeAnchor(node, node->getAnchor(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPivot(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPivot(node, node->getHorizontalPivot(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPivot(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPivot(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPivot(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangePivot(node, node->getPivot(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalPosition(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalPosition(node, node->getHorizontalPosition(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPosition(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalPosition(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalPosition(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangePosition(node, node->getPosition(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalOrigin(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalOrigin(node, node->getHorizontalOrigin(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalOrigin(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalOrigin(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalOrigin(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeVerticalOrigin(node, node->getVerticalOrigin(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeOrigin(SceneTransform node, const glm::vec2& start, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return MakeParallel(
		ChangeHorizontalOrigin(node, start.x, dest.x, duration, easingFunction),
		ChangeVerticalOrigin(node, start.y, dest.y, duration, easingFunction)
	);
}

ActionHelpers::Action ActionHelpers::ChangeOrigin(SceneTransform node, const glm::vec2& dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeOrigin(node, node->getOrigin(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalSize(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalSize(node, node->getHorizontalSize(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalSize(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node, start, dest](float value) {
		node->setVerticalSize(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalSize(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangeSize(node, node->getSize(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalStretch(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalStretch(node, node->getHorizontalStretch(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalStretch(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalStretch(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalStretch(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangeStretch(node, node->getStretch(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setHorizontalScale(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeHorizontalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
		return ChangeHorizontalScale(node, node->getHorizontalScale(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalScale(SceneTransform node, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [node](float value) {
		node->setVerticalScale(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeVerticalScale(SceneTransform node, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([node, dest, duration, easingFunction] {
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
	return Insert([node, dest, duration, easingFunction] {
		return ChangeScale(node, node->getScale(), dest, duration, easingFunction);
	});
}

ActionHelpers::Action ActionHelpers::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float start, float dest, float duration, EasingFunction easingFunction)
{
	return Interpolate(start, dest, duration, easingFunction, [circle](float value) {
		circle->setPie(value);
	});
}

ActionHelpers::Action ActionHelpers::ChangeCirclePie(std::shared_ptr<Scene::Circle> circle, float dest, float duration, EasingFunction easingFunction)
{
	return Insert([circle, dest, duration, easingFunction] {
		return ChangeCirclePie(circle, circle->getPie(), dest, duration, easingFunction);
	});
}
