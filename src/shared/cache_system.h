#pragma once

#include <core/engine.h>
#include <renderer/texture.h>
#include <graphics/font.h>
#include <graphics/image.h>
#include <platform/asset.h>
#include <unordered_map>
#include <memory>
#include <graphics/animation.h>
#include <audio/sound.h>

#define CACHE ENGINE->getSystem<Shared::CacheSystem>()

#define PRECACHE_TEXTURE_ALIAS(NAME, ALIAS) CACHE->loadTexture(NAME, ALIAS)
#define PRECACHE_FONT_ALIAS(NAME, ALIAS) CACHE->loadFont(NAME, ALIAS)

#define PRECACHE_TEXTURE(NAME) PRECACHE_TEXTURE_ALIAS(NAME, NAME)
#define PRECACHE_FONT(NAME) PRECACHE_FONT_ALIAS(NAME, NAME)

#define TEXTURE(NAME) CACHE->getTexture(NAME)
#define FONT(NAME) CACHE->getFont(NAME)
#define ATLAS(NAME, LOAD_FUNC) CACHE->getAtlas(NAME, LOAD_FUNC)
#define ANIMATION(NAME, LOAD_FUNC) CACHE->getAnimation(NAME, LOAD_FUNC)
#define SOUND(NAME) CACHE->getSound(NAME)

namespace Shared 
{
	class CacheSystem
	{
	public:
		using AtlasLoadFunction = std::function<std::shared_ptr<Graphics::Atlas>()>;
		using AnimationLoadFunction = std::function<std::shared_ptr<Graphics::Animation>()>;

	public:
		std::shared_ptr<Renderer::Texture> getTexture(const std::string& name);
		std::shared_ptr<Graphics::Font> getFont(const std::string& name);
		std::shared_ptr<Graphics::Atlas> getAtlas(const std::string& name, AtlasLoadFunction atlasLoadFunction);
		std::shared_ptr<Graphics::Animation> getAnimation(const std::string& name, AnimationLoadFunction animationLoadFunction);
		std::shared_ptr<Audio::Sound> getSound(const std::string& name);
		
		void loadTexture(std::shared_ptr<Renderer::Texture> texture, const std::string& name);
		void loadTexture(std::shared_ptr<Graphics::Image> image, const std::string& name);
		void loadTexture(const std::string& path, const std::string& name);
		void loadTexture(const std::string& path);

		void loadFont(const std::string& path, const std::string& name);
		void loadFont(const std::string& path);

		void loadSound(std::shared_ptr<Audio::Sound> sound, const std::string& name);
		void loadSound(const std::string& path, const std::string& name);
		void loadSound(const std::string& path);

	private:
		std::unordered_map<std::string, std::shared_ptr<Renderer::Texture>> mTextures;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Font>> mFonts;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Atlas>> mAtlases;
		std::unordered_map<std::string, std::shared_ptr<Graphics::Animation>> mAnimations;
		std::unordered_map<std::string, std::shared_ptr<Audio::Sound>> mSounds;
	};
}