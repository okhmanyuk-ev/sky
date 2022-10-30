#include "sound.h"

#include <audio/system.h>

using namespace Audio;

Sound::Sound(const Platform::Asset& asset, bool loop)
{
#ifndef PLATFORM_MAC // TODO: fix for mac
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = static_cast<unsigned int>(asset.getSize());

	FMOD_MODE mode = FMOD_OPENMEMORY;

	if (loop)
		mode |= FMOD_LOOP_NORMAL;
	else
		mode |= FMOD_LOOP_OFF;

	Audio::System::Fmod->createSound((const char*)asset.getMemory(), mode, &exinfo, &sound);
#endif
}

Sound::~Sound()
{
#ifndef PLATFORM_MAC // TODO: fix for mac
	sound->release();
#endif
}
