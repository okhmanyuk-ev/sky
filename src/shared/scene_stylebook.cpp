#include "scene_stylebook.h"
#include <console/device.h>
#include <sky/utils.h>

using namespace Shared;

Stylebook::Stylebook()
{
	auto path = "stylebook.json";
	if (Platform::Asset::Exists(path))
	{
		load(path);
	}
}

void Stylebook::load(const std::string& path_to_json)
{
	auto json_file = Platform::Asset(path_to_json);
	auto json_string = std::string((char*)json_file.getMemory(), json_file.getSize());
	mJson = nlohmann::json::parse(json_string); // TODO: add able to merge multiple jsons
}

void Stylebook::clear()
{
	mJson.clear();
}

void Stylebook::apply(std::shared_ptr<Scene::Node> node, const std::string& name)
{
	if (!mJson.contains(name))
	{
		sky::Log(Console::Color::Red, "style \"" + name + "\" not found");
		return;
	}

	auto item = mJson.at(name);

	if (item.contains("parent"))
		apply(node, item.at("parent"));

	if (item.contains("anchor"))
	{
		auto anchor = item.at("anchor");

		if (auto value = anchor.at(0); !value.is_null())
			node->setHorizontalAnchor(value);

		if (auto value = anchor.at(1); !value.is_null())
			node->setVerticalAnchor(value);
	}

	if (item.contains("pivot"))
	{
		auto pivot = item.at("pivot");

		if (auto value = pivot.at(0); !value.is_null())
			node->setHorizontalPivot(value);

		if (auto value = pivot.at(1); !value.is_null())
			node->setVerticalPivot(value);
	}

	if (item.contains("position"))
	{
		auto position = item.at("position");

		if (auto value = position.at(0); !value.is_null())
			node->setX(value);

		if (auto value = position.at(1); !value.is_null())
			node->setY(value);
	}

	if (item.contains("size"))
	{
		auto size = item.at("size");

		if (auto value = size.at(0); !value.is_null())
			node->setWidth(value);

		if (auto value = size.at(1); !value.is_null())
			node->setHeight(value);
	}
}