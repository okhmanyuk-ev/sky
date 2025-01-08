#pragma once

#include <common/event_system.h>
#include <platform/input.h>
#include <shared/touch_emulator.h>
#include <glm/glm.hpp>
#include <sky/clock.h>
#include <map>

namespace Shared 
{
	class GestureDetector : 
		public Common::Event::Listenable<Platform::Input::Keyboard::Event>,
		public Common::Event::Listenable<TouchEmulator::Event>
	{
	public:
		struct SwipeEvent
		{
			enum class Type
			{
				Left,
				Right,
				Up,
				Down
			};

			Type type;
		};

	public:

	private:
		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const TouchEmulator::Event& e) override;

	private:
		std::map<sky::TimePoint, glm::vec2> mPositions;
	};
}