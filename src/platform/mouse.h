#pragma once

namespace Platform::Mouse
{
	enum class Button
	{
		Left,
		Middle,
		Right
	};

	struct Event
	{
		enum class Type
		{
			ButtonDown,
			ButtonUp,
			Move,
			Wheel
		};

		Type type;
		Button button;

		int x;
		int y;

		float wheelX;
		float wheelY;
	};
}