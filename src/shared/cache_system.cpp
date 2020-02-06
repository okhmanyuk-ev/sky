#include "cache_system.h"

using namespace Shared;

std::shared_ptr<Renderer::Texture> CacheSystem::getTexture(const std::string& name)
{
	loadTexture(name);
	return mTextures.at(name);
}

std::shared_ptr<Graphics::Font> CacheSystem::getFont(const std::string& name)
{
	loadFont(name);
	return mFonts.at(name);
}

void CacheSystem::loadTexture(const Graphics::Image& image, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	mTextures[name] = std::make_shared<Renderer::Texture>(image.getWidth(), image.getHeight(), image.getChannels(), image.getMemory());
}

void CacheSystem::loadTexture(const std::string& path, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	loadTexture(Graphics::Image(Platform::Asset(path)), name);
}

void CacheSystem::loadTexture(const std::string& path)
{
	loadTexture(path, path);
}

void CacheSystem::loadFont(const std::string& path, const std::string& name)
{
	if (mFonts.count(name) > 0)
		return;

	mFonts[name] = std::make_shared<Graphics::Font>(Platform::Asset(path));
}

void CacheSystem::loadFont(const std::string& path)
{
	loadFont(path, path);
}