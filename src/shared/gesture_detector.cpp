#include "gesture_detector.h"
#include <platform/system.h>

using namespace Shared;

void GestureDetector::onEvent(const Platform::Input::Keyboard::Event& e)
{
	if (e.type != Platform::Input::Keyboard::Event::Type::Pressed)
		return;

	if (e.key == Platform::Input::Keyboard::Key::W || e.key == Platform::Input::Keyboard::Key::Up)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Up }));
	else if (e.key == Platform::Input::Keyboard::Key::A || e.key == Platform::Input::Keyboard::Key::Left)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Left }));
	else if (e.key == Platform::Input::Keyboard::Key::S || e.key == Platform::Input::Keyboard::Key::Down)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Down }));
	else if (e.key == Platform::Input::Keyboard::Key::D || e.key == Platform::Input::Keyboard::Key::Right)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Right }));
}

void GestureDetector::onEvent(const TouchEmulator::Event& e)
{	
	auto now = Clock::Now();

	while (!mPositions.empty())
	{
		auto time = mPositions.begin()->first;
		
		if (Clock::ToSeconds(now - time) < 0.125f)
			break;

		mPositions.erase(time);
	}

	auto newest = glm::vec2(static_cast<float>(e.x), static_cast<float>(e.y));

	mPositions[now] = newest;
	
	auto oldest = mPositions.begin()->second;

	if (glm::distance(oldest, newest) < 32.0f * PLATFORM->getScale())
		return;

	auto delta = newest - oldest;

	if (glm::abs(delta.y) > glm::abs(delta.x))
		if (delta.y > 0)
			EVENT->emit(SwipeEvent({ SwipeEvent::Type::Down }));
		else
			EVENT->emit(SwipeEvent({ SwipeEvent::Type::Up }));
	else
		if (delta.x > 0)
			EVENT->emit(SwipeEvent({ SwipeEvent::Type::Right }));
		else
			EVENT->emit(SwipeEvent({ SwipeEvent::Type::Left }));

	mPositions.clear();
}