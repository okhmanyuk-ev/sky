#pragma once

#include <core/engine.h>
#include <audio/sound.h>
#include <fmod/fmod_studio.hpp>

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
		void play(std::shared_ptr<Sound> sound);

	private:
		static inline FMOD::System* Fmod = nullptr;
		static inline FMOD::Studio::System* FmodStudio = nullptr;
		FMOD::Channel* channel = nullptr;
	};

namespace sky
{
	void PlaySound(std::shared_ptr<Audio::Sound> sound);
}