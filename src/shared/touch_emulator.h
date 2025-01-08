#pragma once

#include <platform/input.h>
#include <sky/dispatcher.h>

namespace Shared
{
	class TouchEmulator :
		public sky::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public sky::Listenable<Platform::Input::Mouse::MoveEvent>,
		public sky::Listenable<Platform::Input::Touch::Event>
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
