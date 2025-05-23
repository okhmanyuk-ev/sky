#include "gesture_detector.h"
#include <platform/system.h>
#include <sky/utils.h>

using namespace Shared;

void GestureDetector::onEvent(const Platform::Input::Keyboard::Event& e)
{
	if (e.type != Platform::Input::Keyboard::Event::Type::Pressed)
		return;

	if (e.key == Platform::Input::Keyboard::Key::W || e.key == Platform::Input::Keyboard::Key::Up)
		sky::Emit(SwipeEvent({ SwipeEvent::Type::Up }));
	else if (e.key == Platform::Input::Keyboard::Key::A || e.key == Platform::Input::Keyboard::Key::Left)
		sky::Emit(SwipeEvent({ SwipeEvent::Type::Left }));
	else if (e.key == Platform::Input::Keyboard::Key::S || e.key == Platform::Input::Keyboard::Key::Down)
		sky::Emit(SwipeEvent({ SwipeEvent::Type::Down }));
	else if (e.key == Platform::Input::Keyboard::Key::D || e.key == Platform::Input::Keyboard::Key::Right)
		sky::Emit(SwipeEvent({ SwipeEvent::Type::Right }));
}

void GestureDetector::onEvent(const TouchEmulator::Event& e)
{
	auto now = sky::Now();

	while (!mPositions.empty())
	{
		auto time = mPositions.begin()->first;

		if (sky::ToSeconds(now - time) < 0.125f)
			break;

		mPositions.erase(time);
	}

	auto newest = glm::vec2(e.pos);

	mPositions[now] = newest;

	auto oldest = mPositions.begin()->second;

	if (glm::distance(oldest, newest) < 32.0f * PLATFORM->getScale())
		return;

	auto delta = newest - oldest;

	if (glm::abs(delta.y) > glm::abs(delta.x))
		if (delta.y > 0)
			sky::Emit(SwipeEvent({ SwipeEvent::Type::Down }));
		else
			sky::Emit(SwipeEvent({ SwipeEvent::Type::Up }));
	else
		if (delta.x > 0)
			sky::Emit(SwipeEvent({ SwipeEvent::Type::Right }));
		else
			sky::Emit(SwipeEvent({ SwipeEvent::Type::Left }));

	mPositions.clear();
}
