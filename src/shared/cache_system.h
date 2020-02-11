#pragma once

#include <Core/engine.h>
#include <Renderer/texture.h>
#include <Graphics/font.h>
#include <Graphics/image.h>
#include <Platform/asset.h>
#include <map>
#include <memory>

#define CACHE ENGINE->getSystem<Shared::CacheSystem>()

#define PRECACHE_TEXTURE_ALIAS(NAME, ALIAS) CACHE->loadTexture(NAME, ALIAS)
#define PRECACHE_FONT_ALIAS(NAME, ALIAS) CACHE->loadFont(NAME, ALIAS)

#define PRECACHE_TEXTURE(NAME) PRECACHE_TEXTURE_ALIAS(NAME, NAME)
#define PRECACHE_FONT(NAME) PRECACHE_FONT_ALIAS(NAME, NAME)

#define TEXTURE(NAME) CACHE->getTexture(NAME)
#define FONT(NAME) CACHE->getFont(NAME)

namespace Shared 
{
	class CacheSystem
	{
	public:
		std::shared_ptr<Renderer::Texture> getTexture(const std::string& name);
		std::shared_ptr<Graphics::Font> getFont(const std::string& name);

		void loadTexture(const Graphics::Image& image, const std::string& name);
		void loadTexture(const std::string& path, const std::string& name);
		void loadTexture(const std::string& path);

		void loadFont(const std::string& path, const std::string& name);
		void loadFont(const std::string& path);

	private:
		std::map<std::string, std::shared_ptr<Renderer::Texture>> mTextures;
		std::map<std::string, std::shared_ptr<Graphics::Font>> mFonts;
	};
}