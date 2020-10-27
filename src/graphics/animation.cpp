#include "animation.h"
#include <nlohmann/json.hpp>

using namespace Graphics;

void Animation::SaveToFile(const std::string& path, const Animation& animation, Platform::Asset::Path pathType)
{
	Atlas::SaveToFile(path, animation.getAtlas(), pathType);
	auto json = nlohmann::json();
	for (const auto& [name, regions] : animation.getStates())
	{
		for (const auto& region : regions)
		{
			json[name].push_back(region);
		}
	}
	auto json_dump = json.dump(4);
	Platform::Asset::Write(path + "_animation.json", json_dump.data(), json_dump.size(), pathType);
}

Animation Animation::OpenFromFile(const std::string& image_path, const std::string& atlas_path,
	const std::string& animation_path, Platform::Asset::Path path_type)
{
	auto atlas = Atlas::OpenFromFile(image_path, atlas_path, path_type);
	auto json_file = Platform::Asset(animation_path, path_type);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto states = Animation::StatesMap();
	for (const auto& [name, regions] : json.items())
	{
		for (const auto& region : regions)
		{
			states[name].push_back(region.get<std::string>());
		}
	}
	return Animation(atlas, states);
}

Animation Animation::OpenFromFile(const std::string& smart_path, Platform::Asset::Path path_type)
{
	auto image_path = smart_path + ".png";
	auto atlas_path = smart_path + "_atlas.json";
	auto animation_path = smart_path + "_animation.json";
	return OpenFromFile(image_path, atlas_path, animation_path, path_type);
}

Animation::Animation(const Atlas& atlas, const StatesMap& states) : mAtlas(atlas), mStates(states)
{
	//
}