#pragma once

#include <graphics/image.h>
#include <graphics/tex_region.h>
#include <map>
#include <memory>
#include <tuple>

namespace Graphics
{
	class Atlas
	{
	public:
		using Images = std::map<std::string, Image>;
		using Regions = std::map<std::string, TexRegion>;

	public:
		static void SaveToFile(const std::string& path, const Regions& regions,
			sky::Asset::Storage storage = sky::Asset::Storage::Assets);

		static std::tuple<Image, Regions> MakeFromImages(const Images& images, bool anti_bleeding = true);
		static Image MakeAntibleedImage(const Image& image);

	public:
		Atlas(const Regions& regions);
		Atlas(const sky::Asset& regions_file);

	private:
		static Regions ParseRegionsFromFile(const sky::Asset& file);

	public:
		const auto& getRegions() const { return mRegions; }

	private:
		Regions mRegions;
	};
}