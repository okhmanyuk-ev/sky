#pragma once

#include <platform/input.h>
#include <common/event_system.h>

namespace Shared
{
	class TouchEmulator :
		public Common::Event::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public Common::Event::Listenable<Platform::Input::Mouse::MoveEvent>,
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
			glm::ivec2 pos;
		};

	private:
		void onEvent(const Platform::Input::Mouse::ButtonEvent& e) override;
		void onEvent(const Platform::Input::Mouse::MoveEvent& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

	private:
		bool mMouseDown = false;
	};
}
