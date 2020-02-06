#pragma once

#include <Graphics/image.h>
#include <Graphics/tex_region.h>
#include <map>
#include <optional>

namespace Graphics
{
	class Atlas
	{
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
