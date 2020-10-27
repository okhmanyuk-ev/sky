#include "cache_system.h"
#include <console/device.h>

using namespace Shared;

std::shared_ptr<Renderer::Texture> CacheSystem::getTexture(const std::string& name)
{
	loadTexture(name);
	
	if (mTextures.count(name) == 0)
		return nullptr;
	
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

std::shared_ptr<Audio::Sound> CacheSystem::getSound(const std::string& name)
{
	loadSound(name);

	if (mSounds.count(name) == 0)
		return nullptr;

	return mSounds.at(name);
}

void CacheSystem::loadTexture(std::shared_ptr<Renderer::Texture> texture, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	mTextures[name] = texture;
}

void CacheSystem::loadTexture(std::shared_ptr<Graphics::Image> image, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	auto texture = std::make_shared<Renderer::Texture>(image->getWidth(), image->getHeight(), image->getChannels(), image->getMemory());

	loadTexture(texture, name);
}

void CacheSystem::loadTexture(const std::string& path, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	if (!Platform::Asset::Exists(path))
	{
		CONSOLE_DEVICE->writeLine("cannot find texture: " + path, Console::Color::Red);
		return;
	}

	auto image = std::make_shared<Graphics::Image>(path);

	loadTexture(image, name);
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

	auto animation = std::make_shared<Graphics::Animation>(Graphics::Animation::OpenFromFile(path));

	loadAnimation(animation, name);
}

void CacheSystem::loadAnimation(const std::string& path)
{
	loadAnimation(path, path);
}

void CacheSystem::loadSound(std::shared_ptr<Audio::Sound> sound, const std::string& name)
{
	if (mSounds.count(name) > 0)
		return;

	mSounds[name] = sound;
}

void CacheSystem::loadSound(const std::string& path, const std::string& name)
{
	if (mSounds.count(name) > 0)
		return;

	if (!Platform::Asset::Exists(path))
	{
		CONSOLE_DEVICE->writeLine("cannot find sound: " + path, Console::Color::Red);
		return;
	}

	auto sound = std::make_shared<Audio::Sound>(path);

	loadSound(sound, name);
}

void CacheSystem::loadSound(const std::string& path)
{
	loadSound(path, path);
}
