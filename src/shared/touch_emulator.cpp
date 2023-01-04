#include "touch_emulator.h"

using namespace Shared;

void TouchEmulator::onEvent(const Platform::Input::Mouse::ButtonEvent& e)
{
	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Pressed && !mMouseDown)
	{
		mMouseDown = true;
		EVENT->emit(Event{
			.type = Event::Type::Begin,
			.x = e.pos.x,
			.y = e.pos.y
		});
	}
	else if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Released && mMouseDown)
	{
		mMouseDown = false;
		EVENT->emit(Event{
			.type = Event::Type::End,
			.x = e.pos.x,
			.y = e.pos.y
		});
	}
}

void TouchEmulator::onEvent(const Platform::Input::Mouse::MoveEvent& e)
{
	if (!mMouseDown)
		return;

	EVENT->emit(Event{
		.type = Event::Type::Continue,
		.x = e.pos.x,
		.y = e.pos.y
	});
}

void TouchEmulator::onEvent(const Platform::Input::Touch::Event& e)
{
	auto fwd = Event();
	fwd.x = e.x;
	fwd.y = e.y;
	
	if (e.type == Platform::Input::Touch::Event::Type::Begin)
		fwd.type = Event::Type::Begin;
	else if (e.type == Platform::Input::Touch::Event::Type::Continue)
		fwd.type = Event::Type::Continue;
	else
		fwd.type = Event::Type::End;

	EVENT->emit(fwd);
}
