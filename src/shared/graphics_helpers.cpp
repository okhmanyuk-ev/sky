#include "graphics_helpers.h"

#include <stb_image_write.h>
#include <nlohmann/json.hpp>

using namespace Shared;

struct SaveImageContext
{
	std::string path;
	Platform::Asset::Path pathType;
};

void GraphicsHelpers::SaveAtlasToFile(const std::string& path, const Graphics::Atlas& atlas, Platform::Asset::Path pathType)
{
	auto writeFunc = [](void* context, void* memory, int size) {
		auto saveImageContext = static_cast<SaveImageContext*>(context);
		Platform::Asset::Write(saveImageContext->path + ".png", memory, size, saveImageContext->pathType);
	};
	const auto& image = atlas.getImage();

	SaveImageContext context;
	context.path = path;
	context.pathType = pathType;
	
	stbi_write_png_to_func(writeFunc, &context, image.getWidth(), image.getHeight(), 
		image.getChannels(), image.getMemory(), image.getWidth() * image.getChannels());

	auto json = nlohmann::json();
	for (const auto& [name, region] : atlas.getTexRegions())
	{
		json[name] = { (int)region.pos.x, (int)region.pos.y, (int)region.size.x, (int)region.size.y };
	}
	auto json_dump = json.dump(4);

	Platform::Asset::Write(path + "_atlas.json", json_dump.data(), json_dump.size(), pathType);
}

Graphics::Atlas GraphicsHelpers::OpenAtlasFromFile(const std::string& path, Platform::Asset::Path pathType)
{
	auto png_file = Platform::Asset(path + ".png", pathType);
	auto json_file = Platform::Asset(path + "_atlas.json", pathType);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto regions = Graphics::Atlas::TexRegionMap();
	for (const auto& [key, value]  : json.items())
	{
		regions.insert({ key, { { value[0], value[1] }, { value[2], value[3] } } });
	}
	return Graphics::Atlas(png_file, regions);
}

void GraphicsHelpers::SaveAnimationToFile(const std::string& path, const Graphics::Animation& animation, Platform::Asset::Path pathType)
{
	SaveAtlasToFile(path, animation.getAtlas(), pathType);
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

Graphics::Animation GraphicsHelpers::OpenAnimationFromFile(const std::string& path, Platform::Asset::Path pathType)
{
	auto atlas = OpenAtlasFromFile(path, pathType);
	auto json_file = Platform::Asset(path + "_animation.json", pathType);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	auto json = nlohmann::json::parse(json_string);
	auto states = Graphics::Animation::StatesMap();
	for (const auto& [name, regions] : json.items())
	{
		for (const auto& region : regions)
		{
			states[name].push_back(region.get<std::string>());
		}
	}
	return Graphics::Animation(atlas, states);
}