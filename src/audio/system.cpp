#include "system.h"

#if defined(PLATFORM_ANDROID)
#include <Platform/system_android.h>
#endif
#include <cassert>

using namespace Audio;

FMOD::System* System::Fmod = nullptr;

System::System()
{
	FMOD::System_Create(&Fmod);
	unsigned int version;
	Fmod->getVersion(&version);
	assert(version == FMOD_VERSION);
#if defined(PLATFORM_ANDROID)
	JNIEnv* pEnv = nullptr;
	Platform::SystemAndroid::Instance->activity->vm->AttachCurrentThread(&pEnv, nullptr);
#endif
	auto result = Fmod->init(32, FMOD_INIT_NORMAL, nullptr);
	assert(result == FMOD_RESULT::FMOD_OK);
#if defined(PLATFORM_ANDROID)
	Platform::SystemAndroid::Instance->activity->vm->DetachCurrentThread();
#endif
}

void System::update()
{
	Fmod->update();
}

void System::play(const Sound& sound)
{
	Fmod->playSound(sound.sound, nullptr, false, &channel);
}

System::~System()
{
	Fmod->release();
}