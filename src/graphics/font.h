#pragma once

#include <sky/asset.h>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include <skygfx/skygfx.h>

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
			glm::vec2 pos;
			glm::vec2 size;
			glm::vec2 offset;
			float xadvance;
		};

	public:
		Font(void* data, size_t size);
		Font(const sky::Asset& asset);
		~Font();

		auto getTexture() const { return mTexture; }
		const Glyph& getGlyph(wchar_t symbol) const;

		static float getScaleFactorForSize(float size);

		float getKerning(wchar_t left, wchar_t right) const;

		float getAscent() const { return mAscent; }
		float getDescent() const { return mDescent; }
		float getLinegap() const { return mLinegap; }

	private:
		std::shared_ptr<skygfx::Texture> mTexture = nullptr;
		std::unordered_map<wchar_t, Glyph> mGlyphs;
		std::unordered_map<wchar_t, std::unordered_map<wchar_t, float>> mKernings;
		float mAscent = 0.0f;
		float mDescent = 0.0f;
		float mLinegap = 0.0f;
	};
}
