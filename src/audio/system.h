#pragma once

#include <Core/engine.h>
#include <Audio/sound.h>
#include <fmod/fmod.hpp>

#define AUDIO ENGINE->getSystem<Audio::System>()

namespace Audio
{
	class System
	{
		friend class Sound;
	public:
		System();
		~System();

	public:
		void update();

	public:
		void play(const Sound& sound);

	private:
		static FMOD::System* Fmod;
		FMOD::Channel* channel = nullptr;
	};
}