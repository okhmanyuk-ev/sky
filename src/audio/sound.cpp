#include "sound.h"

#include <Audio/system.h>

using namespace Audio;

Sound::Sound(const Platform::Asset& asset, bool loop)
{
/*	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = static_cast<unsigned int>(asset.getSize());

	FMOD_MODE mode = FMOD_OPENMEMORY;

	if (loop)
		mode |= FMOD_LOOP_NORMAL;
	else
		mode |= FMOD_LOOP_OFF;

	Audio::System::Fmod->createSound((const char*)asset.getMemory(), mode, &exinfo, &sound);*/
}

Sound::~Sound()
{
	//sound->release();
}
