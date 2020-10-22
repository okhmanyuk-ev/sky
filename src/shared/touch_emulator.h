#pragma once

#include <platform/input.h>
#include <common/event_system.h>

namespace Shared 
{
	class TouchEmulator : 
		public Common::Event::Listenable<Platform::Input::Mouse::Event>,
		public Common::Event::Listenable<Platform::Input::Touch::Event>
	{
	public:
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

	private:
		void onEvent(const Platform::Input::Mouse::Event& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

	private:
		bool mMouseDown = false;
	};
}