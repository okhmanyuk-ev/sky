#pragma once

#include <Platform/asset.h>
#include <Renderer/texture.h>
#include <unordered_map>
#include <memory>
#include <tinyutf8.hpp>

namespace Graphics
{
	class Font
	{
	public:
		static inline const float GlyphSize = 32.0f;
		static inline const float SdfPadding = GlyphSize / 8.0f;
		static inline const float SdfOnedge = 0.5f;

	public:
		struct Glyph
		{
			int x; // source coords
			int y;
			int w; 
			int h; 
			float xoff;
			float yoff;
			float xadvance;
		};

	public:
		Font(void* data, size_t size);
		Font(const Platform::Asset& asset);
		~Font();

		const auto getTexture() const { return mTexture; }
		const Glyph& getGlyph(uint16_t symbol) const;

		static float getScaleFactorForSize(float size);

		float getStringWidth(utf8_string::iterator begin, utf8_string::iterator end, float size = GlyphSize) const;
		float getStringWidth(const utf8_string& text, float size = GlyphSize) const;

		float getStringHeight(utf8_string::iterator begin, utf8_string::iterator end, float size = GlyphSize) const;
		float getStringHeight(const utf8_string& text, float size = GlyphSize) const;

		float getKerning(uint16_t left, uint16_t right) const;

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		std::unordered_map<uint16_t, Glyph> mGlyphs;
		std::unordered_map<uint16_t, std::unordered_map<uint16_t, float>> mKernings;
	};
}