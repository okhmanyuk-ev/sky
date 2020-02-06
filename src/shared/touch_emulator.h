#pragma once

#include <Platform/mouse.h>
#include <Platform/touch.h>
#include <Common/event_system.h>

namespace Shared 
{
	class TouchEmulator : 
		public Common::EventSystem::Listenable<Platform::Mouse::Event>,
		public Common::EventSystem::Listenable<Platform::Touch::Event>
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
		void event(const Platform::Mouse::Event& e) override;
		void event(const Platform::Touch::Event& e) override;

	private:
		bool mMouseDown = false;
	};
}