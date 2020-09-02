#pragma once

#include <common/event_system.h>
#include <platform/input.h>
#include <shared/touch_emulator.h>
#include <glm/glm.hpp>
#include <core/clock.h>
#include <map>

namespace Shared 
{
	class GestureDetector : public Common::EventSystem::Listenable<Platform::Input::Keyboard::Event>,
		public Common::EventSystem::Listenable<TouchEmulator::Event>
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
		void event(const Platform::Input::Keyboard::Event& e) override;
		void event(const TouchEmulator::Event& e) override;

	private:
		std::map<Clock::TimePoint, glm::vec2> mPositions;
	};
}