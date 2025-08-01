#pragma once

#include <skygfx/skygfx.h>
#include <graphics/all.h>
#include <unordered_map>
#include <memory>
#include <set>
#include <optional>
#include <sky/audio.h>
#include <nlohmann/json.hpp>

namespace sky
{
	class Cache
	{
	public:
		Graphics::TexturePart getTexture(const std::string& name);
		std::shared_ptr<Graphics::Font> getFont(const std::string& name);
		std::shared_ptr<Audio::Sound> getSound(const std::string& name);
		std::shared_ptr<Graphics::Atlas> getAtlas(const std::string& name);
		std::shared_ptr<Graphics::Animation> getAnimation(const std::string& name);
		const nlohmann::json& getJson(const std::string& name);

	public:
		bool hasTexture(const std::string& name) const;

		void loadTexture(std::shared_ptr<skygfx::Texture> texture, const std::string& name);
		void loadTexture(const Graphics::Image& image, const std::string& name);
		void loadTexture(const std::string& path, std::optional<std::string> name = std::nullopt);

		void loadFont(const std::string& path, std::optional<std::string> name = std::nullopt);

		void loadSound(std::shared_ptr<Audio::Sound> sound, const std::string& name);
		void loadSound(const std::string& path, std::optional<std::string> name = std::nullopt);

		void loadAtlas(const std::string& path, std::optional<std::string> name = std::nullopt);

		void loadAnimation(const std::string& path, std::optional<std::string> name = std::nullopt);

		void loadJson(const std::string& path, std::optional<std::string> name = std::nullopt);

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
		std::unordered_map<std::string, Graphics::TexturePart> mTextureParts;
	};
}