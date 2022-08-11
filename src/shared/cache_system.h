#pragma once

#include <core/engine.h>
#include <renderer/all.h>
#include <graphics/all.h>
#include <platform/asset.h>
#include <unordered_map>
#include <memory>
#include <set>
#include <audio/sound.h>
#include <nlohmann/json.hpp>

#define CACHE ENGINE->getSystem<Shared::CacheSystem>()

#define PRECACHE_TEXTURE_ALIAS(NAME, ALIAS) CACHE->loadTexture(NAME, ALIAS)
#define PRECACHE_FONT_ALIAS(NAME, ALIAS) CACHE->loadFont(NAME, ALIAS)

#define PRECACHE_TEXTURE(NAME) PRECACHE_TEXTURE_ALIAS(NAME, NAME)
#define PRECACHE_FONT(NAME) PRECACHE_FONT_ALIAS(NAME, NAME)

#define TEXTURE(NAME) CACHE->getTexture(NAME)
#define FONT(NAME) CACHE->getFont(NAME)
#define SOUND(NAME) CACHE->getSound(NAME)
#define ATLAS(NAME) CACHE->getAtlas(NAME)
#define ANIMATION(NAME) CACHE->getAnimation(NAME)
#define JSON(NAME) CACHE->getJson(NAME)

namespace Shared 
{
	class CacheSystem
	{
	public:
		Graphics::TexCell getTexture(const std::string& name);
		std::shared_ptr<Graphics::Font> getFont(const std::string& name);
		std::shared_ptr<Audio::Sound> getSound(const std::string& name);
		std::shared_ptr<Graphics::Atlas> getAtlas(const std::string& name);
		std::shared_ptr<Graphics::Animation> getAnimation(const std::string& name);
		const nlohmann::json& getJson(const std::string& name);

	public:
		void loadTexture(std::shared_ptr<skygfx::Texture> texture, const std::string& name);
		void loadTexture(std::shared_ptr<Graphics::Image> image, const std::string& name);
		void loadTexture(const std::string& path, const std::string& name);
		void loadTexture(const std::string& path);

		void loadFont(const std::string& path, const std::string& name);
		void loadFont(const std::string& path);

		void loadSound(std::shared_ptr<Audio::Sound> sound, const std::string& name);
		void loadSound(const std::string& path, const std::string& name);
		void loadSound(const std::string& path);

		void loadAtlas(const std::string& path, const std::string& name);
		void loadAtlas(const std::string& path);

		void loadAnimation(const std::string& path, const std::string& name);
		void loadAnimation(const std::string& path);

		void loadJson(const std::string& path, const std::string& name);
		void loadJson(const std::string& path);

	public:
		void makeAtlas(const std::string& name, const std::set<std::string>& paths);
		void makeAtlases();

	private:
		std::unordered_map<std::string, std::shared_ptr<skygfx::Texture>> mTextures;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Font>> mFonts;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Atlas>> mAtlases;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Animation>> mAnimations;
		std::unordered_map<std::string, std::shared_ptr<Audio::Sound>> mSounds;
		std::unordered_map<std::string, nlohmann::json> mJsons;

	private:
		std::unordered_map<std::string, Graphics::TexCell> mTexCells;
	};
}