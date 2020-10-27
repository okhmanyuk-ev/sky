#pragma once

#include <renderer/texture.h>
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
		static void SaveToFile(const std::string& path, const Image& image, const Regions& regions, 
			Platform::Asset::Path pathType = Platform::Asset::Path::Relative);
		
		static Atlas OpenFromFile(const std::string& image_path, const std::string& atlas_path, 
			Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

		static std::tuple<Image, Regions> MakeFromImages(const Images& images);

	public:
		Atlas(const Image& image, const Regions& tex_regions);

	public:
		auto getTexture() const { return mTexture; }
		const auto& getRegions() const { return mRegions; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		Regions mRegions;
	};
}