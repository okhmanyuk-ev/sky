#pragma once

#include <graphics/atlas.h>
#include <graphics/animation.h>

namespace Shared::GraphicsHelpers
{
	// save atlas to assets
	void SaveAtlasToFile(const std::string& path, const Graphics::Atlas& atlas, Platform::Asset::Path pathType = Platform::Asset::Path::Relative);

	// load atlas from assets
	Graphics::Atlas OpenAtlasFromFile(const std::string& image_path, const std::string& atlas_path, 
		Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

	// save animation to assets
	void SaveAnimationToFile(const std::string& path, const Graphics::Animation& animation, Platform::Asset::Path pathType = Platform::Asset::Path::Relative);

	// open animation from assets
	Graphics::Animation OpenAnimationFromFile(const std::string& image_path, const std::string& atlas_path,
		const std::string& animation_path, Platform::Asset::Path path_type = Platform::Asset::Path::Relative);

	Graphics::Animation OpenAnimationFromFile(const std::string& smart_path, 
		Platform::Asset::Path path_type = Platform::Asset::Path::Relative);
}
