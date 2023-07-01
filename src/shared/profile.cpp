#include "profile.h"
#include <platform/system.h>
#include <platform/asset.h>
#include <console/device.h>
#include <common/task_system.h>
#include <common/event_system.h>
#include <common/frame_system.h>
#include <common/helpers.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace Shared;

void Profile::load()
{
#ifndef EMSCRIPTEN
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
		sky::Log(Console::Color::Red, e.what());
		sky::Log(Console::Color::Green, "making new profile");
		makeDefault();
		return;
	}
#else
	auto str = emscripten_run_script_string("localStorage.getItem('profile');");
	try
	{
		auto json = nlohmann::json::parse(str);
		read(json);
	}
	catch (const std::exception& e)
	{
		LOGC(e.what(), Console::Color::Red);
		LOGC("making new profile", Console::Color::Green);
		makeDefault();
		return;
	}
#endif
}

void Profile::save()
{
#ifndef EMSCRIPTEN
	mSaveMutex.lock();
	auto json = nlohmann::json();
	write(json);
	auto bson = nlohmann::json::to_bson(json);
	Platform::Asset::Write("save.bson", bson.data(), bson.size(), Platform::Asset::Storage::Bundle);
	mSaveMutex.unlock();
#else
	auto json = nlohmann::json();
	write(json);
	auto str = json.dump();
	emscripten_run_script(fmt::format("localStorage.setItem('profile', '{}');", str).c_str());
#endif
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
