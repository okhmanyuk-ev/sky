#include "system.h"

#if defined(PLATFORM_ANDROID)
#include <platform/system_android.h>
#endif
#include <cassert>

using namespace Audio;

System::System()
{
	FMOD::Studio::System::create(&FmodStudio);
	FmodStudio->getCoreSystem(&Fmod);
#if defined(PLATFORM_ANDROID)
	Platform::SystemAndroid::BeginEnv();
#endif
	FmodStudio->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, NULL);
#if defined(PLATFORM_ANDROID)
	Platform::SystemAndroid::EndEnv();
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

	Fmod->playSound(sound->sound, nullptr, false, &channel);
}

System::~System()
{
	FmodStudio->release();
}