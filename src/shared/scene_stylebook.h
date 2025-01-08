#pragma once

#include <scene/all.h>
#include <nlohmann/json.hpp>

#define STYLEBOOK sky::Locator<Shared::Stylebook>::GetService()

namespace Shared
{
	class Stylebook
	{
	public:
		Stylebook();

	public:
		void load(const std::string& path_to_json);
		void clear();

		void apply(std::shared_ptr<Scene::Node> node, const std::string& name);

	private:
		nlohmann::json mJson;
	};
}