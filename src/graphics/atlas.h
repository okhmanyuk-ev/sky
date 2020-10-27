#pragma once

#include <graphics/image.h>
#include <graphics/tex_region.h>
#include <map>
#include <optional>

namespace Graphics
{
	class Atlas
	{
	public:
		static void SaveToFile(const std::string& path, const Atlas& atlas, 
			Platform::Asset::Path pathType = Platform::Asset::Path::Relative);
		
		static Atlas OpenFromFile(const std::string& image_path, const std::string& atlas_path, 
			Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

	public:
		using TexRegionMap = std::map<std::string, TexRegion>;

	public:
		Atlas(const std::map<std::string, Graphics::Image>& images);
		Atlas(const Graphics::Image& image, const TexRegionMap& tex_regions);

	public:
		const auto& getImage() const { return mImage.value(); }
		const auto& getTexRegions() const { return mTexRegions; }

	private:
		std::optional<Graphics::Image> mImage;
		TexRegionMap mTexRegions;
	};
}