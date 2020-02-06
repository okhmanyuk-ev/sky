#pragma once

#include <Common/event_system.h>
#include <Platform/keyboard.h>
#include <Shared/touch_emulator.h>
#include <glm/glm.hpp>
#include <Core/clock.h>

namespace Shared 
{
	class GestureDetector : public Common::EventSystem::Listenable<Platform::Keyboard::Event>,
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
		void event(const Platform::Keyboard::Event& e) override;
		void event(const TouchEmulator::Event& e) override;

	private:
		std::map<Clock::TimePoint, glm::vec2> mPositions;
	};
}