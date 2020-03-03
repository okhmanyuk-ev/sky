#pragma once

#include <platform/asset.h>
#include <fmod/fmod.hpp>

namespace Audio
{
	class Sound
	{
		friend class System;

	public:
		Sound(const Platform::Asset& asset, bool loop = false);
		~Sound();

	private:
		FMOD::Sound* sound;
	};
}
