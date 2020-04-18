#include "cache_system.h"
#include "graphics_helpers.h"

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

std::shared_ptr<Graphics::Animation> CacheSystem::getAnimation(const std::string& name)
{
	loadAnimation(name);
	return mAnimations.at(name);
}

void CacheSystem::loadTexture(std::shared_ptr<Graphics::Image> image, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	mTextures[name] = std::make_shared<Renderer::Texture>(image->getWidth(), image->getHeight(), image->getChannels(), image->getMemory());
}

void CacheSystem::loadTexture(const std::string& path, const std::string& name)
{
	if (mAnimations.count(name) > 0)
		return;

	loadTexture(std::make_shared<Graphics::Image>(path), name);
}

void CacheSystem::loadTexture(const std::string& path)
{
	loadTexture(path, path);
}

void CacheSystem::loadFont(const std::string& path, const std::string& name)
{
	if (mFonts.count(name) > 0)
		return;

	mFonts[name] = std::make_shared<Graphics::Font>(path);
}

void CacheSystem::loadFont(const std::string& path)
{
	loadFont(path, path);
}

void CacheSystem::loadAnimation(std::shared_ptr<Graphics::Animation> animation, const std::string& name)
{
	if (mAnimations.count(name) > 0)
		return;

	mAnimations[name] = animation;
}

void CacheSystem::loadAnimation(const std::string& path, const std::string& name)
{
	if (mAnimations.count(name) > 0)
		return;

	loadAnimation(std::make_shared<Graphics::Animation>(Shared::GraphicsHelpers::OpenAnimationFromFile(path)), name);
}

void CacheSystem::loadAnimation(const std::string& path)
{
	loadAnimation(path, path);
}