#include "animation.h"
#include <nlohmann/json.hpp>

using namespace Graphics;

void Animation::SaveToFile(const std::string& path, const Image& image, const States& states, Platform::Asset::Path pathType)
{
	auto json = nlohmann::json();
	for (const auto& [name, regions] : states)
	{
		for (const auto& region : regions)
		{
			json[name].push_back(region);
		}
	}
	auto json_dump = json.dump(4);
	Platform::Asset::Write(path + "_animation.json", json_dump.data(), json_dump.size(), pathType);
}

Animation::Animation(const States& states) : 
	mStates(states)
{
	//
}

Animation::Animation(const Platform::Asset& states_file) : 
	Animation(ParseStatesFromFile(states_file))
{
	//
}

Animation::States Animation::ParseStatesFromFile(const Platform::Asset& file)
{
	auto json_string = std::string((char*)file.getMemory(), file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto states = States();
	for (const auto& [name, regions] : json.items())
	{
		for (const auto& region : regions)
		{
			states[name].push_back(region.get<std::string>());
		}
	}
	return states;
}