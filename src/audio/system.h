#pragma once

#include <sky/locator.h>
#include <audio/sound.h>
#include <fmod/fmod_studio.hpp>

#define AUDIO sky::Locator<Audio::System>::GetService()

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
}

namespace sky
{
	void PlaySound(std::shared_ptr<Audio::Sound> sound);
}