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
		
		static std::tuple<Image, Regions> MakeFromImages(const Images& images);

	public:
		Atlas(std::shared_ptr<Renderer::Texture> texture, const Regions& regions);
		Atlas(std::shared_ptr<Renderer::Texture> texture, const Platform::Asset& regions_file);

	private:
		static Regions ParseRegionsFromFile(const Platform::Asset& file);

	public:
		auto getTexture() const { return mTexture; }
		const auto& getRegions() const { return mRegions; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		Regions mRegions;
	};
}