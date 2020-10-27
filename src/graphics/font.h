#pragma once

#include <platform/asset.h>
#include <renderer/texture.h>
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

		auto getTexture() const { return mTexture; }
		const Glyph& getGlyph(utf8_string::value_type symbol) const;

		static float getScaleFactorForSize(float size);

		float getStringWidth(utf8_string::iterator begin, utf8_string::iterator end, float size = GlyphSize) const;
		float getStringWidth(const utf8_string& text, float size = GlyphSize) const;

		float getKerning(utf8_string::value_type left, utf8_string::value_type right) const;

		float getAscent() const { return mAscent; }
		float getDescent() const { return mDescent; }
		float getLinegap() const { return mLinegap; }

		float getCustomVerticalOffset() const { return mCustomVerticalOffset; }
		void setCustomVerticalOffset(float value) { mCustomVerticalOffset = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		std::unordered_map<utf8_string::value_type, Glyph> mGlyphs;
		std::unordered_map<utf8_string::value_type, std::unordered_map<utf8_string::value_type, float>> mKernings;
		float mAscent = 0.0f;
		float mDescent = 0.0f;
		float mLinegap = 0.0f;
		float mCustomVerticalOffset = 0.0f; // TODO: should be removed, code must work without this
	};
}