#include "cache.h"
#include <nlohmann/json.hpp>
#include <common/helpers.h>
#include <sky/utils.h>
#include <sky/asset.h>

Graphics::TexCell sky::Cache::getTexture(const std::string& name)
{
	if (mTexCells.count(name) > 0)
		return mTexCells.at(name);

	loadTexture(name);

	if (mTextures.count(name) == 0)
		return { nullptr, Graphics::TexRegion() };

	return { mTextures.at(name), Graphics::TexRegion() };
}

std::shared_ptr<Graphics::Font> sky::Cache::getFont(const std::string& name)
{
	loadFont(name);
	return mFonts.at(name);
}

std::shared_ptr<sky::Audio::Sound> sky::Cache::getSound(const std::string& name)
{
	loadSound(name);

	if (mSounds.count(name) == 0)
		return nullptr;

	return mSounds.at(name);
}

std::shared_ptr<Graphics::Atlas> sky::Cache::getAtlas(const std::string& name)
{
	loadAtlas(name);
	return mAtlases.at(name);
}

std::shared_ptr<Graphics::Animation> sky::Cache::getAnimation(const std::string& name)
{
	loadAnimation(name);
	return mAnimations.at(name);
}

const nlohmann::json& sky::Cache::getJson(const std::string& name)
{
	loadJson(name);
	return mJsons.at(name);
}

bool sky::Cache::hasTexture(const std::string& name) const
{
	return mTextures.contains(name);
}

void sky::Cache::loadTexture(std::shared_ptr<skygfx::Texture> texture, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	mTextures[name] = texture;
}

void sky::Cache::loadTexture(const Graphics::Image& image, const std::string& name)
{
	if (mTextures.count(name) > 0)
		return;

	assert(image.getChannels() == 4); // TODO: skygfx::Format::Byte(1/2/3)

	auto texture = std::make_shared<skygfx::Texture>(image.getWidth(), image.getHeight(),
		skygfx::PixelFormat::RGBA8UNorm, image.getMemory());

	loadTexture(texture, name);
}

void sky::Cache::loadTexture(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mTextures.count(name) > 0)
		return;

	if (!sky::Asset::Exists(path))
	{
		sky::Log(Console::Color::Red, "cannot find texture: " + path);
		return;
	}

	auto image = Graphics::Image(path);

	loadTexture(image, name);
}

void sky::Cache::loadFont(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mFonts.count(name) > 0)
		return;

	mFonts[name] = std::make_shared<Graphics::Font>(path);
}

void sky::Cache::loadSound(std::shared_ptr<Audio::Sound> sound, const std::string& name)
{
	if (mSounds.count(name) > 0)
		return;

	mSounds[name] = sound;
}

void sky::Cache::loadSound(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mSounds.count(name) > 0)
		return;

	if (!sky::Asset::Exists(path))
	{
		sky::Log(Console::Color::Red, "cannot find sound: " + path);
		return;
	}

	auto sound = std::make_shared<Audio::Sound>(path);
	loadSound(sound, name);
}

void sky::Cache::loadAtlas(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mAtlases.count(name) > 0)
		return;

	mAtlases[name] = std::make_shared<Graphics::Atlas>(path);
}

void sky::Cache::loadAnimation(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mAnimations.count(name) > 0)
		return;

	mAnimations[name] = std::make_shared<Graphics::Animation>(path);
}

void sky::Cache::loadJson(const std::string& path, std::optional<std::string> _name)
{
	auto name = _name.value_or(path);

	if (mJsons.count(name) > 0)
		return;

	if (!sky::Asset::Exists(path))
		return;

	mJsons[name] = Common::Helpers::LoadJsonFromAsset({ path });
}

void sky::Cache::makeAtlas(const std::string& name, const std::set<std::string>& paths)
{
	Graphics::Atlas::Images images;

	for (const auto& path : paths)
	{
		images.insert({ path, Graphics::Image(sky::Asset(path)) });
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

void sky::Cache::makeAtlases()
{
	auto json = getJson("atlases.json");

	for (auto field : json.items())
	{
		auto name = field.key();
		auto paths = field.value();
		makeAtlas(name, paths);
	}
}
