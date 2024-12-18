#include "cache_system.h"
#include <console/device.h>
#include <nlohmann/json.hpp>
#include <common/helpers.h>

using namespace Shared;

Graphics::TexCell CacheSystem::getTexture(const std::string& name)
{
	if (mTexCells.count(name) > 0)
		return mTexCells.at(name);

	loadTexture(name);

	if (mTextures.count(name) == 0)
		return { nullptr, Graphics::TexRegion() };

	return { mTextures.at(name), Graphics::TexRegion() };
}

std::shared_ptr<Graphics::Font> CacheSystem::getFont(const std::string& name)
{
	loadFont(name);
	return mFonts.at(name);
}

std::shared_ptr<Audio::Sound> CacheSystem::getSound(const std::string& name)
{
	loadSound(name);

	if (mSounds.count(name) == 0)
		return nullptr;

	return mSounds.at(name);
}

std::shared_ptr<Graphics::Atlas> CacheSystem::getAtlas(const std::string& name)
{
	loadAtlas(name);
	return mAtlases.at(name);
}

std::shared_ptr<Graphics::Animation> CacheSystem::getAnimation(const std::string& name)
{
	loadAnimation(name);
	return mAnimations.at(name);
}

const nlohmann::json& CacheSystem::getJson(const std::string& name)
{
	loadJson(name);
	return mJsons.at(name);
}

bool CacheSystem::hasTexture(const std::string& name) const
{
	return mTextures.contains(name);
}

void CacheSystem::loadTexture(std::shared_ptr<skygfx::Texture> texture, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	mTextures[name] = texture;
}

void CacheSystem::loadTexture(const Graphics::Image& image, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	assert(image.getChannels() == 4); // TODO: skygfx::Format::Byte(1/2/3)

	auto texture = std::make_shared<skygfx::Texture>(image.getWidth(), image.getHeight(),
		skygfx::PixelFormat::RGBA8UNorm, image.getMemory());

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

	auto image = Graphics::Image(path);

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

void CacheSystem::loadAtlas(const std::string& path, const std::string& name)
{
	if (mAtlases.count(name) > 0)
		return;

	mAtlases[name] = std::make_shared<Graphics::Atlas>(path);
}

void CacheSystem::loadAtlas(const std::string& path)
{
	loadAtlas(path, path);
}

void CacheSystem::loadAnimation(const std::string& path, const std::string& name)
{
	if (mAnimations.count(name) > 0)
		return;

	mAnimations[name] = std::make_shared<Graphics::Animation>(path);
}

void CacheSystem::loadAnimation(const std::string& path)
{
	loadAnimation(path, path);
}

void CacheSystem::loadJson(const std::string& path, const std::string& name)
{
	if (mJsons.count(name) > 0)
		return;

	if (!Platform::Asset::Exists(path))
		return;

	mJsons[name] = Common::Helpers::LoadJsonFromAsset({ path });
}

void CacheSystem::loadJson(const std::string& path)
{
	loadJson(path, path);
}

void CacheSystem::makeAtlas(const std::string& name, const std::set<std::string>& paths)
{
	Graphics::Atlas::Images images;

	for (const auto& path : paths)
	{
		images.insert({ path, Graphics::Image(Platform::Asset(path)) });
	}

	auto [image, regions] = Graphics::Atlas::MakeFromImages(images);
	auto _image = Graphics::Image(image);
	loadTexture(_image, name);

	auto texture = getTexture(name);

	for (const auto& [name, tex_region] : regions)
	{
		mTexCells.insert({ name, Graphics::TexCell(texture, tex_region) });
	}
}

void CacheSystem::makeAtlases()
{
	auto json = JSON("atlases.json");

	for (auto field : json.items())
	{
		auto name = field.key();
		auto paths = field.value();
		makeAtlas(name, paths);
	}
}
