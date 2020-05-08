#include "scene_stylebook.h"
#include <console/device.h>

using namespace Shared;

void Stylebook::load(const std::string& path_to_json)
{
	auto json_file = Platform::Asset(path_to_json);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	mJson = nlohmann::json::parse(json_string);
}

void Stylebook::clear()
{
	mJson.clear();
}

void Stylebook::apply(Scene::Node& node, const std::string& name)
{
	if (!mJson.contains(name))
	{
		LOGC("style \"" + name + "\" not found", Console::Color::Red);
		return;
	}

	auto item = mJson.at(name);

	if (item.contains("position"))
	{
		auto position = item.at("position");
		node.setPosition({ position[0].get<float>(), position[1].get<float>() });
	}
}

void Stylebook::apply(std::shared_ptr<Scene::Node> node, const std::string& name)
{
	apply(*node, name);
}