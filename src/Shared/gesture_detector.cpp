#include "gesture_detector.h"
#include <Platform/system.h>

using namespace Shared;

void GestureDetector::event(const Platform::Keyboard::Event& e)
{
	if (e.type != Platform::Keyboard::Event::Type::Pressed)
		return;

	if (e.key == Platform::Keyboard::Key::W || e.key == Platform::Keyboard::Key::Up)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Up }));
	else if (e.key == Platform::Keyboard::Key::A || e.key == Platform::Keyboard::Key::Left)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Left }));
	else if (e.key == Platform::Keyboard::Key::S || e.key == Platform::Keyboard::Key::Down)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Down }));
	else if (e.key == Platform::Keyboard::Key::D || e.key == Platform::Keyboard::Key::Right)
		EVENT->emit(SwipeEvent({ SwipeEvent::Type::Right }));
}

void GestureDetector::event(const TouchEmulator::Event& e)
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