#include "profile.h"
#include <platform/system.h>
#include <sky/asset.h>
#include <sky/console.h>
#include <sky/threadpool.h>
#include <sky/utils.h>
#include <sky/scheduler.h>
#include <common/helpers.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace Shared;

void Profile::load()
{
#ifndef EMSCRIPTEN
	auto path = "save.bson";

	if (!sky::Asset::Exists(path, sky::Asset::Storage::Bundle))
	{
		makeDefault();
		return;
	}

	try
	{
		auto json = Common::Helpers::LoadBsonFromAsset(sky::Asset(path, sky::Asset::Storage::Bundle));
		read(json);
	}
	catch (const std::exception& e)
	{
		sky::Log(sky::Console::Color::Red, e.what());
		sky::Log(sky::Console::Color::Green, "making new profile");
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
		sky::Log(sky::Console::Color::Red, e.what());
		sky::Log(sky::Console::Color::Green, "making new profile");
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
	sky::Asset::Write("save.bson", bson.data(), bson.size(), sky::Asset::Storage::Bundle);
	mSaveMutex.unlock();
#else
	auto json = nlohmann::json();
	write(json);
	auto str = json.dump();
	emscripten_run_script(fmt::format("localStorage.setItem('profile', '{}');", str).c_str());
#endif
	sky::Emit(ProfileSavedEvent());
}

void Profile::clear()
{
	sky::Emit(ProfileClearedEvent());
	makeDefault();
}
