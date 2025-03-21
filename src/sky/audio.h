#pragma once

#include <memory>
#if 0
#include <fmod/fmod_studio.hpp>
#endif
#include <sky/asset.h>

namespace sky
{
	class Audio
	{
	public:
		class Sound;

	public:
		Audio();
		~Audio();

	public:
		void play(std::shared_ptr<Sound> sound);

#if 0
	private:
		static inline FMOD::System* Fmod = nullptr;
		static inline FMOD::Studio::System* FmodStudio = nullptr;
		FMOD::Channel* channel = nullptr;
#endif
	};

	class Audio::Sound
	{
		friend class Audio;

	public:
		Sound(const sky::Asset& asset, bool loop = false);
		~Sound();

#if 0
	private:
		FMOD::Sound* sound;
#endif
	};
}