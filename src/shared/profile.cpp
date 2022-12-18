#include "profile.h"
#include <platform/system.h>
#include <platform/asset.h>
#include <console/device.h>
#include <common/task_system.h>
#include <common/event_system.h>
#include <common/frame_system.h>
#include <common/helpers.h>

using namespace Shared;

void Profile::load()
{
	auto path = "save.bson";

	if (!Platform::Asset::Exists(path, Platform::Asset::Storage::Bundle))
	{
		makeDefault();
		return;
	}

	try
	{
		auto json = Common::Helpers::LoadBsonFromAsset(Platform::Asset(path, Platform::Asset::Storage::Bundle));
		read(json);
	}
	catch (const std::exception& e)
	{
		LOGC(e.what(), Console::Color::Red);
		LOGC("making new profile", Console::Color::Green);
		makeDefault();
		return;
	}
}

void Profile::save()
{
	mSaveMutex.lock();
	auto json = nlohmann::json();
	write(json);
	auto bson = nlohmann::json::to_bson(json);
	Platform::Asset::Write("save.bson", bson.data(), bson.size(), Platform::Asset::Storage::Bundle);
	mSaveMutex.unlock();
	FRAME->addOneThreadsafe([] {
		EVENT->emit(ProfileSavedEvent());
	});
}

void Profile::clear()
{
	EVENT->emit(ProfileClearedEvent());
	makeDefault();
}

void Profile::saveAsync()
{
#ifndef EMSCRIPTEN
	TASK->addTask([this] {
		save();
	});
#else
	save();
#endif
}