#include "animation.h"
#include <nlohmann/json.hpp>
#include <filesystem>

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

Animation Animation::OpenFromFile(const std::string& image_path, const std::string& atlas_path,
	const std::string& animation_path, Platform::Asset::Path path_type)
{
	auto atlas = Atlas::OpenFromFile(image_path, atlas_path, path_type);
	auto json_file = Platform::Asset(animation_path, path_type);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto states = Animation::States();
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

#if defined(PLATFORM_WINDOWS)
std::tuple<Animation::States, Image, Atlas::Regions> Animation::MakeFromFolder(const std::string& _path)
{
	auto path = std::filesystem::path(_path);
	auto folders = std::filesystem::directory_iterator(path);

	auto states = States();
	auto images = Atlas::Images();

	for (auto folder : folders)
	{
		if (!folder.is_directory())
			continue;

		auto files = std::filesystem::directory_iterator(folder.path());

		for (auto file : files)
		{
			if (!file.is_regular_file())
				continue;

			auto ext = file.path().extension().string();

			if (ext != ".png")
				continue;

			auto folderName = folder.path().filename().string();
			auto fileName = folderName + "_" + file.path().filename().replace_extension().string();

			images.insert({ fileName, Image({ file.path().string(), Platform::Asset::Path::Absolute }) });

			states[folderName].push_back(fileName);
		}
	}

	auto [image, regions] = Atlas::MakeFromImages(images);

	return { states, image, regions };
}
#endif

Animation::Animation(const Atlas& atlas, const States& states) : mAtlas(atlas), mStates(states)
{
	//
}