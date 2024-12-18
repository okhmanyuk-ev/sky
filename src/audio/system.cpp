#include "system.h"

#if defined(PLATFORM_ANDROID)
#include <platform/system_android.h>
#endif
#include <cassert>
#include <console/device.h>

using namespace Audio;

System::System()
{
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
}

void System::update()
{
	//Fmod->update(); // TODO: why everything working without it ?
}

void System::play(std::shared_ptr<Sound> sound)
{
	if (sound == nullptr)
		return;

#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	Fmod->playSound(sound->sound, nullptr, false, &channel);
#endif
}

System::~System()
{
#if !defined(PLATFORM_MAC) & !defined(EMSCRIPTEN)
	FmodStudio->release();
#endif
}

void sky::PlaySound(std::shared_ptr<Audio::Sound> sound)
{
	if (!ENGINE->hasSystem<Audio::System>())
	{
		Log("PlaySound: cannot find audio system");
		return;
	}

	AUDIO->play(sound);
}
