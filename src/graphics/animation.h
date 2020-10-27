#pragma once
#include "atlas.h"
#include <vector>

namespace Graphics
{
	class Animation
	{
	public:
		using Frames = std::vector<std::string>;
		using StatesMap = std::map<std::string, Frames>;

	public:
		static void SaveToFile(const std::string& path, const Animation& animation, Platform::Asset::Path pathType = Platform::Asset::Path::Relative);

		static Animation OpenFromFile(const std::string& image_path, const std::string& atlas_path,
			const std::string& animation_path, Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

		static Animation OpenFromFile(const std::string& smart_path,
			Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

	public:
		Animation(const Atlas& atlas, const StatesMap& states);

	public:
		const auto& getAtlas() const { return mAtlas; }
		const auto& getStates() const { return mStates; }

	private:
		Atlas mAtlas;
		StatesMap mStates;
	};
}