#pragma once

namespace Platform::Touch
{
	struct Event
	{
		enum class Type
		{
			Begin,
			Continue,
			End
		};

		Type type;

		int x;
		int y;
	};
}