#include "animation.h"
#include <nlohmann/json.hpp>

using namespace Graphics;

void Animation::SaveToFile(const std::string& path, const Image& image, const Atlas::Regions& regions, 
	const States& states, Platform::Asset::Path pathType)
{
	Atlas::SaveToFile(path, image, regions, pathType);
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

Animation::Animation(std::shared_ptr<Atlas> atlas, const States& states) : 
	mAtlas(atlas), mStates(states)
{
	//
}

Animation::Animation(std::shared_ptr<Atlas> atlas, const Platform::Asset& states_file) : 
	Animation(atlas, ParseStatesFromFile(states_file))
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