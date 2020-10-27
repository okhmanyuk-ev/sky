#pragma once
#include "atlas.h"
#include <vector>

namespace Graphics
{
	class Animation
	{
	public:
		using Frames = std::vector<std::string>;
		using States = std::map<std::string, Frames>;

	public:
		static void SaveToFile(const std::string& path, const Image& image, const Atlas::Regions& regions,
			const States& states, Platform::Asset::Path pathType = Platform::Asset::Path::Relative);

		static Animation OpenFromFile(const std::string& image_path, const std::string& atlas_path,
			const std::string& animation_path, Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

		static Animation OpenFromFile(const std::string& smart_path,
			Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

		// TODO: to enable crossplatform, make with Platform::Asset, not <filesystem>
#if defined(PLATFORM_WINDOWS)
		static std::tuple<States, Image, Atlas::Regions> MakeFromFolder(const std::string& path);
#endif

	public:
		Animation(const Atlas& atlas, const States& states);

	public:
		const auto& getAtlas() const { return mAtlas; }
		const auto& getStates() const { return mStates; }

	private:
		Atlas mAtlas;
		States mStates;
	};
}