#include "font.h"

#include <stb_truetype.h>
#include <rectpack2D/finders_interface.h>
#include <Graphics/image.h>
#include <vector>

using namespace Graphics;

Font::Font(void* data, size_t size) 
{
	stbtt_fontinfo info;
	stbtt_InitFont(&info, (uint8_t*)data, 0);

	float scale = stbtt_ScaleForPixelHeight(&info, GlyphSize);

	struct glyph
	{
		int w;
		int h;
		int xoff;
		int yoff;
		unsigned char* pixels; // TODO: memory leak
	};

	auto glyphs = std::vector<glyph>(info.numGlyphs);

	for (int i = 0; i < info.numGlyphs; i++)
	{
		const int Onedge = int(SdfOnedge * 255.0f);
		const float PixelDistScale = Onedge / SdfPadding;

		auto& g = glyphs[i];
		g.pixels = stbtt_GetGlyphSDF(&info, scale, i, (int)SdfPadding, Onedge, PixelDistScale, &g.w, &g.h, &g.xoff, &g.yoff);
	}

	using namespace rectpack2D;
	constexpr bool allow_flip = false;
	const auto runtime_flipping_mode = flipping_option::DISABLED;
	using spaces_type = empty_spaces<allow_flip, default_empty_spaces>;
	using rect_type = output_rect_t<spaces_type>;

	std::vector<rect_type> rectangles;

	for (const auto& g : glyphs)
	{
		rectangles.emplace_back(rect_xywh(0, 0, g.w, g.h));
	}

	auto report_successful = [](rect_type&) {
		return callback_result::CONTINUE_PACKING;
	};

	auto report_unsuccessful = [](rect_type&) {
		return callback_result::ABORT_PACKING;
	};

	const auto max_side = 1 << 12;
	const auto discard_step = 1;

	auto my_custom_order_1 = [](const auto a, const auto b) {
		return a->get_wh().pathological_mult() > b->get_wh().pathological_mult();
	};

	auto my_custom_order_2 = [](const auto a, const auto b) {
		return a->get_wh().pathological_mult() < b->get_wh().pathological_mult();
	};

	const auto result_size = find_best_packing<spaces_type>(rectangles,
		make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode), my_custom_order_1, my_custom_order_2);

	for (int i = 0; i < info.numGlyphs; i++)
	{
		int xadvance;
		stbtt_GetGlyphHMetrics(&info, i, &xadvance, nullptr);
		mGlyphs[i].x = rectangles[i].x;
		mGlyphs[i].y = rectangles[i].y;
		mGlyphs[i].w = rectangles[i].w;
		mGlyphs[i].h = rectangles[i].h;
		mGlyphs[i].xoff = static_cast<float>(glyphs[i].xoff);
		mGlyphs[i].yoff = static_cast<float>(glyphs[i].yoff);
		mGlyphs[i].xadvance = static_cast<float>(xadvance) * scale;
	}

	for (int i = 0; i < info.numGlyphs; i++)
	{
		for (int j = 0; j < info.numGlyphs; j++)
		{
			mKernings[i][j] = stbtt_GetGlyphKernAdvance(&info, i, j) * scale;
		}
	}

	const auto dst_width = result_size.w;
	const auto dst_height = result_size.h;
	const int channels = 4;
	auto image = Graphics::Image(dst_width, dst_height, channels);

	for (int i = 0; i < info.numGlyphs; i++)
	{
		auto& r = rectangles[i];
		auto& g = glyphs[i];
		for (int x = 0; x < r.w; x++)
		{
			for (int y = 0; y < r.h; y++)
			{
				auto pixel = &((uint8_t*)image.getMemory())[((r.x + (r.y * dst_width)) * channels) + ((x + (y * dst_width)) * channels)];
				memset(pixel, g.pixels[x + (y * r.w)], channels);
			}
		}
	}

	mTexture = std::make_shared<Renderer::Texture>(image.getWidth(), image.getHeight(), image.getChannels(), image.getMemory());

	// codepoints

	for (int i = 0; i < 65535; i++)
	{
		auto index = stbtt_FindGlyphIndex(&info, i);
		
		if (index == 0)
			continue;

		mGlyphIndices[i] = index;
	}
}

Font::Font(const Platform::Asset& asset) : Font(asset.getMemory(), asset.getSize())
{
	//
}

Font::~Font()
{
	//
}

float Font::getScaleFactorForSize(float size)
{
	return size / GlyphSize;
}

const Font::Glyph& Font::getGlyph(uint16_t symbol) const
{
	return mGlyphs.at(getGlyphIndex(symbol));
}

float Font::getStringWidth(const std::wstring& text, float size) const
{
	float result = 0.0f;
	for (int i = 0; i < static_cast<int>(text.size()); i++)
	{
		result += getGlyph(text.at(i)).xadvance;
	
		if (i < static_cast<int>(text.size()) - 1)
		{
			result += getKerning(text.at(i), text.at(i + 1));
		}
	}
	return result * Font::getScaleFactorForSize(size);
}

float Font::getStringHeight(const std::wstring& text, float size) const
{
	float result = 0.0f;
	for (const auto& symbol : text)
	{
		float h = static_cast<float>(getGlyph(symbol).h);

		if (result >= h)
			continue;

		result = h;
	}
	return (result - (SdfPadding * 2.0f)) * Font::getScaleFactorForSize(size);
}

float Font::getKerning(uint16_t left, uint16_t right) const
{
	auto left_glyph = getGlyphIndex(left);
	auto right_glyph = getGlyphIndex(right);

	if (mKernings.count(left_glyph) == 0)
		return 0.0f;

	if (mKernings.at(left_glyph).count(right_glyph) == 0)
		return 0.0f;

	return mKernings.at(left_glyph).at(right_glyph);
}

int Font::getGlyphIndex(uint16_t symbol) const
{
	if (mGlyphIndices.count(symbol) == 0)
		return 0;

	return mGlyphIndices.at(symbol); // 433 == 'ÿ' (1103)
}