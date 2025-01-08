#include "touch_emulator.h"
#include <sky/utils.h>

using namespace Shared;

void TouchEmulator::onEvent(const Platform::Input::Mouse::ButtonEvent& e)
{
	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Pressed && !mMouseDown)
	{
		mMouseDown = true;
		sky::Emit(Event{
			.type = Event::Type::Begin,
			.pos = e.pos
		});
	}
	else if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Released && mMouseDown)
	{
		mMouseDown = false;
		sky::Emit(Event{
			.type = Event::Type::End,
			.pos = e.pos
		});
	}
}

void TouchEmulator::onEvent(const Platform::Input::Mouse::MoveEvent& e)
{
	if (!mMouseDown)
		return;

	sky::Emit(Event{
		.type = Event::Type::Continue,
		.pos = e.pos
	});
}

void TouchEmulator::onEvent(const Platform::Input::Touch::Event& e)
{
	const static std::unordered_map<Platform::Input::Touch::Event::Type, Event::Type> TypeMap = {
		{ Platform::Input::Touch::Event::Type::Begin, Event::Type::Begin },
		{ Platform::Input::Touch::Event::Type::Continue, Event::Type::Continue },
		{ Platform::Input::Touch::Event::Type::End, Event::Type::End }
	};

	sky::Emit(Event{
		.type = TypeMap.at(e.type),
		.pos = e.pos
	});
}
