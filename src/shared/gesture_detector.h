#pragma once

#include <sky/event.h>
#include <platform/input.h>
#include <shared/touch_emulator.h>
#include <glm/glm.hpp>
#include <sky/clock.h>
#include <map>

namespace Shared
{
	class GestureDetector : public sky::Listenable<Platform::Input::Keyboard::Event>,
		public sky::Listenable<TouchEmulator::Event>
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