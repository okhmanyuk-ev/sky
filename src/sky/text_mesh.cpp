#include "text_mesh.h"
#include <sky/color.h>

using namespace sky;

TextMesh::Symbol::Symbol(glm::vec2 pos, glm::vec2 size, float line_y) :
	pos(pos), size(size), line_y(line_y)
{
}

TextMesh::TextMesh(skygfx::Topology topology, Vertices vertices, Indices indices, Symbols symbols, glm::vec2 size) :
	topology(topology),
	vertices(std::move(vertices)),
	indices(std::move(indices)),
	symbols(std::move(symbols)),
	size(size)
{
}

void TextMesh::setSymbolColor(size_t index, const glm::vec4& color)
{
	size_t base_vtx = index * 4;

	for (size_t i = base_vtx; i < base_vtx + 4; i++)
	{
		vertices[i].color = color;
	}
}

static float GetStringWidth(const Graphics::Font& font, std::wstring::const_iterator begin, std::wstring::const_iterator end, float size = Graphics::Font::GlyphSize)
{
	float result = 0.0f;

	for (auto it = begin; it != end; ++it)
	{
		result += font.getGlyph(*it).xadvance;

		if (it != end - 1)
		{
			result += font.getKerning(*it, *(it + 1));
		}
	}
	return result * Graphics::Font::getScaleFactorForSize(size);
}

static float GetAlignNormalizedValue(TextMesh::Align align)
{
	if (align == TextMesh::Align::Center)
		return 0.5f;

	if (align == TextMesh::Align::Right)
		return 1.0f;

	return 0.0f;
}

struct Line
{
	Line(const Graphics::Font& font, std::wstring::const_iterator begin, std::wstring::const_iterator end) :
		begin(begin),
		end(end)
	{
		width = GetStringWidth(font, begin, end);
	}
	std::wstring::const_iterator begin;
	std::wstring::const_iterator end;
	float width;
};

static TextMesh CreateTextMesh(const Graphics::Font& font, const std::vector<Line>& lines, float size, TextMesh::Align align, float minWidth = 0.0f)
{
	const auto texture = font.getTexture();

	glm::vec2 tex_size = { static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()) };

	size_t length = 0;
	float width = 0.0f;

	for (const auto& line : lines)
	{
		length += std::distance(line.begin, line.end);
		width = glm::max(width, line.width);
	}

	width = glm::max(width, minWidth);

	auto vertices = TextMesh::Vertices(length * 4);
	auto indices = TextMesh::Indices(length * 6);
	TextMesh::Symbols symbols;

	int i = 0;
	float height = 0.0f;

	for (const auto& line : lines)
	{
		float pos_x = (width - line.width) * GetAlignNormalizedValue(align);

		for (auto it = line.begin; it != line.end; ++it, i++)
		{
			const auto& glyph = font.getGlyph(*it);

			auto vtx = &vertices[i * 4];
			auto idx = &indices[i * 6];

			pos_x += glyph.offset.x;
			float pos_y = height + font.getAscent() + glyph.offset.y;

			glm::vec2 p1 = { pos_x, pos_y };
			glm::vec2 p2 = glm::vec2{ pos_x, pos_y } + glyph.size;

			pos_x -= glyph.offset.x;
			pos_x += glyph.xadvance;

			if (it != line.end - 1)
			{
				pos_x += font.getKerning(*it, *(it + 1));
			}

			auto uv1 = glyph.pos / tex_size;
			auto uv2 = (glyph.pos + glyph.size) / tex_size;

			auto color = sky::GetColor<glm::vec4>(sky::Color::White);

			vtx[0] = { { p1.x, p1.y, 0.0f }, color, { uv1.x, uv1.y } };
			vtx[1] = { { p1.x, p2.y, 0.0f }, color, { uv1.x, uv2.y } };
			vtx[2] = { { p2.x, p2.y, 0.0f }, color, { uv2.x, uv2.y } };
			vtx[3] = { { p2.x, p1.y, 0.0f }, color, { uv2.x, uv1.y } };

			auto base_vtx = i * 4;

			idx[0] = base_vtx + 0;
			idx[1] = base_vtx + 1;
			idx[2] = base_vtx + 2;
			idx[3] = base_vtx + 0;
			idx[4] = base_vtx + 2;
			idx[5] = base_vtx + 3;

			symbols.push_back(TextMesh::Symbol(p1, glyph.size, height));
		}

		height += font.getAscent() - font.getDescent() + font.getLinegap();
	}

	height -= font.getLinegap();
	height *= font.getScaleFactorForSize(size);

	width *= font.getScaleFactorForSize(size);

	return TextMesh(skygfx::Topology::TriangleList, std::move(vertices), std::move(indices), std::move(symbols), { width, height });
}

TextMesh TextMesh::createTextMesh(const Graphics::Font& font, std::wstring::const_iterator begin,
	std::wstring::const_iterator end, float size, Align align)
{
	std::vector<Line> lines;

	auto it = begin;

	while (it != end)
	{
		if (*it == '\n')
		{
			lines.push_back(Line(font, begin, it));
			it++;
			begin = it;
			continue;
		}

		it++;
	}

	if (begin != end)
		lines.push_back(Line(font, begin, end));

	return CreateTextMesh(font, lines, size, align);
}

TextMesh TextMesh::createTextMesh(const Graphics::Font& font, const std::wstring& text, float size, Align align)
{
	return createTextMesh(font, text.begin(), text.end(), size, align);
}

TextMesh TextMesh::createWordWrapTextMesh(const Graphics::Font& font, const std::wstring& text,
	float maxWidth, float size, Align align)
{
	float unscaledMaxWidth = maxWidth / font.getScaleFactorForSize(size);

	std::vector<Line> lines;

	auto begin = text.begin();
	auto it = begin;

	while (it != text.end())
	{
		std::advance(it, 1);

		if (it != text.end() && *it == '\n')
		{
			lines.push_back(Line(font, begin, it));
			begin = it + 1;
			continue;
		}

		auto length = std::distance(begin, it);

		if (length <= 1)
			continue;

		auto str_w = GetStringWidth(font, begin, it);

		if (str_w <= unscaledMaxWidth)
			continue;

		--it;

		auto best_it = it;

		while (best_it != begin)
		{
			if (*best_it == ' ')
			{
				++best_it;
				break;
			}

			--best_it;
		}

		if (best_it != begin)
			it = best_it;

		lines.push_back(Line(font, begin, it));
		begin = it;
	}

	if (begin != text.end())
		lines.push_back(Line(font, begin, text.end()));

	return CreateTextMesh(font, lines, size, align, unscaledMaxWidth);
}
