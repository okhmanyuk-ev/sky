#include "touch_emulator.h"

using namespace Shared;

void TouchEmulator::event(const Platform::Mouse::Event& e)
{
	if (e.type == Platform::Mouse::Event::Type::Wheel)
		return;

	auto fwd = Event();
	fwd.x = e.x;
	fwd.y = e.y;

	if (e.type == Platform::Mouse::Event::Type::ButtonDown && !mMouseDown)
	{
		fwd.type = Event::Type::Begin;
		mMouseDown = true;
		EVENT->emit(fwd);
	}
	else if (e.type == Platform::Mouse::Event::Type::Move && mMouseDown)
	{
		fwd.type = Event::Type::Continue;
		EVENT->emit(fwd);
	}
	else if (e.type == Platform::Mouse::Event::Type::ButtonUp && mMouseDown)
	{
		fwd.type = Event::Type::End;
		mMouseDown = false;
		EVENT->emit(fwd);
	}
}

void TouchEmulator::event(const Platform::Touch::Event& e)
{
	auto fwd = Event();
	fwd.x = e.x;
	fwd.y = e.y;
	
	if (e.type == Platform::Touch::Event::Type::Begin)
		fwd.type = Event::Type::Begin;
	else if (e.type == Platform::Touch::Event::Type::Continue)
		fwd.type = Event::Type::Continue;
	else
		fwd.type = Event::Type::End;

	EVENT->emit(fwd);
}