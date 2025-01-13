#pragma once

#include <memory>
#include <fmod/fmod_studio.hpp>
#include <platform/asset.h>

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

	private:
		static inline FMOD::System* Fmod = nullptr;
		static inline FMOD::Studio::System* FmodStudio = nullptr;
		FMOD::Channel* channel = nullptr;
	};

	class Audio::Sound
	{
		friend class Audio;

	public:
		Sound(const Platform::Asset& asset, bool loop = false);
		~Sound();

	private:
		FMOD::Sound* sound;
	};
}