#include "profile.h"
#include <platform/system.h>
#include <platform/asset.h>
#include <console/device.h>
#include <common/task_system.h>
#include <common/event_system.h>

using namespace Shared;

void Profile::load()
{
	auto path = PLATFORM->getAppFolder() + "save.bson";

	if (!Platform::Asset::Exists(path, Platform::Asset::Path::Absolute))
	{
		makeDefault();
		return;
	}

	auto json_file = Platform::Asset(path, Platform::Asset::Path::Absolute);
	nlohmann::json json;
	
	try
	{
		json = nlohmann::json::from_bson(std::string((char*)json_file.getMemory(), json_file.getSize()));
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
	Platform::Asset::Write(PLATFORM->getAppFolder() + "save.bson", bson.data(), bson.size(), Platform::Asset::Path::Absolute);
	mSaveMutex.unlock();
}

void Profile::clear()
{
	EVENT->emit(ProfileClearedEvent());
	makeDefault();
}

void Profile::saveAsync()
{
	TASK->addTask([this] {
		save();
	});
}