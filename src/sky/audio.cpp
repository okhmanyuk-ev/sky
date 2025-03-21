#include "audio.h"

using namespace sky;

Audio::Audio()
{
#if 0
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	FMOD::Studio::System::create(&FmodStudio);
	FmodStudio->getCoreSystem(&Fmod);
#if defined(PLATFORM_ANDROID)
	Platform::SystemAndroid::BeginEnv();
#endif
	FmodStudio->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, NULL);
#if defined(PLATFORM_ANDROID)
	Platform::SystemAndroid::EndEnv();
#endif
#endif
#endif
}

Audio::~Audio()
{
#if 0
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	FmodStudio->release();
#endif
#endif
}

void Audio::play(std::shared_ptr<Sound> sound)
{
	if (sound == nullptr)
		return;

#if 0
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	Fmod->playSound(sound->sound, nullptr, false, &channel);
#endif
#endif
}

Audio::Sound::Sound(const sky::Asset& asset, bool loop)
{
#if 0
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = static_cast<unsigned int>(asset.getSize());

	FMOD_MODE mode = FMOD_OPENMEMORY;

	if (loop)
		mode |= FMOD_LOOP_NORMAL;
	else
		mode |= FMOD_LOOP_OFF;

	Audio::Fmod->createSound((const char*)asset.getMemory(), mode, &exinfo, &sound);
#endif
#endif
}

Audio::Sound::~Sound()
{
#if 0
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	sound->release();
#endif
#endif
}
