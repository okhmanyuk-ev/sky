#pragma once

#include <Platform/asset.h>
#include <Renderer/texture.h>
#include <map>
#include <memory>

namespace Graphics
{
	class Font
	{
	public:
		static inline const float GlyphSize = 64.0f;
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
		const Glyph& getGlyph(int c) const;

		static float getScaleFactorForSize(float size);

		float getStringWidth(const std::string& text, float size = GlyphSize) const;
		float getStringHeight(const std::string& text, float size = GlyphSize) const;

		float getKerning(int left, int right) const;

	private:
		int getGlyphIndex(int codepoint) const;

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		std::map<int, Glyph> mGlyphs;
		std::map<int, std::map<int, float>> mKernings;
		std::map<int, int> mCodepoints;
	};
}