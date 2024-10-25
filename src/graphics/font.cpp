#include "font.h"

#include <stb_truetype.h>
#include <rectpack2D/finders_interface.h>
#include <graphics/image.h>
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

	const auto finder_input = make_finder_input(max_side, discard_step, report_successful, report_unsuccessful, runtime_flipping_mode);
	const auto result_size = find_best_packing<spaces_type>(rectangles, finder_input);

	for (int i = 0; i < 65535; i++)
	{
		auto g_index = stbtt_FindGlyphIndex(&info, i);

		if (g_index == 0 && i != 0)
			continue;

		int xadvance;
		stbtt_GetCodepointHMetrics(&info, i, &xadvance, nullptr);
		auto& _glyph = mGlyphs[i];
		const auto& rect = rectangles.at(g_index);
		const auto& g = glyphs.at(g_index);
		_glyph.pos.x = static_cast<float>(rect.x);
		_glyph.pos.y = static_cast<float>(rect.y);
		_glyph.size.x = static_cast<float>(rect.w);
		_glyph.size.y = static_cast<float>(rect.h);
		_glyph.offset.x = static_cast<float>(g.xoff);
		_glyph.offset.y = static_cast<float>(g.yoff);
		_glyph.xadvance = static_cast<float>(xadvance) * scale;
	}

	for (const auto& [left_index, left_glyph] : mGlyphs)
	{
		for (const auto& [right_index, right_glyph] : mGlyphs)
		{
			auto kern = stbtt_GetCodepointKernAdvance(&info, left_index, right_index);

			if (kern == 0.0f)
				continue;

			mKernings[left_index][right_index] = kern * scale;
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
				auto pixel = image.getPixel(r.x + x, r.y + y);
				memset(pixel, g.pixels[x + (y * r.w)], channels);
			}
		}
	}

	mTexture = std::make_shared<skygfx::Texture>(image.getWidth(), image.getHeight(), skygfx::PixelFormat::RGBA8UNorm,
		image.getMemory());

	int ascent = 0;
	int descent = 0;
	int linegap = 0;

	auto res = stbtt_GetFontVMetricsOS2(&info, &ascent, &descent, &linegap);

	assert(res == 1);

	mAscent = ascent * scale;
	mDescent = descent * scale;
	mLinegap = linegap * scale;
}

Font::Font(const Platform::Asset& asset) : Font(asset.getMemory(), asset.getSize())
{
}

Font::~Font()
{
}

float Font::getScaleFactorForSize(float size)
{
	return size / GlyphSize;
}

const Font::Glyph& Font::getGlyph(wchar_t symbol) const
{
	if (mGlyphs.count(symbol) == 0)
		return mGlyphs.at(0);

	return mGlyphs.at(symbol);
}

float Font::getStringWidth(std::wstring::const_iterator begin, std::wstring::const_iterator end, float size) const
{
	float result = 0.0f;

	for (auto it = begin; it != end; ++it)
	{
		result += getGlyph(*it).xadvance;

		if (it != end - 1)
		{
			result += getKerning(*it, *(it + 1));
		}
	}
	return result * Font::getScaleFactorForSize(size);
}

float Font::getStringWidth(const std::wstring& text, float size) const
{
	return getStringWidth(text.begin(), text.end(), size);
}

float Font::getKerning(wchar_t left, wchar_t right) const
{
	if (mKernings.count(left) == 0)
		return 0.0f;

	const auto& left_kern = mKernings.at(left);

	if (left_kern.count(right) == 0)
		return 0.0f;

	return left_kern.at(right);
}
