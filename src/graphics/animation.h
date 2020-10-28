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

		// TODO: to enable crossplatform, make with Platform::Asset, not <filesystem>
#if defined(PLATFORM_WINDOWS)
		static std::tuple<States, Image, Atlas::Regions> MakeFromFolder(const std::string& path);
#endif

	public:
		Animation(std::shared_ptr<Atlas> atlas, const Platform::Asset& anim_file);

	public:
		auto getAtlas() const { return mAtlas; }
		const auto& getStates() const { return mStates; }

	private:
		std::shared_ptr<Atlas> mAtlas;
		States mStates;
	};
}