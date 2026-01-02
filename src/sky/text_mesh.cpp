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

static std::vector<Line> CreateLines(const Graphics::Font& font, const std::wstring& text, std::optional<float> maxWidth, float size)
{
	std::vector<Line> result;

	auto begin = text.begin();
	auto end = text.end();
	auto it = begin;

	while (it != end)
	{
		if (*it == '\n')
		{
			result.push_back(Line(font, begin, it));
			++it;
			begin = it;
			continue;
		}

		++it;

		if (!maxWidth.has_value())
			continue;

		auto length = std::distance(begin, it);

		if (length <= 1)
			continue;

		auto str_w = GetStringWidth(font, begin, it, size);

		if (str_w <= maxWidth.value())
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

		result.push_back(Line(font, begin, it));
		begin = it;
	}

	result.push_back(Line(font, begin, end));

	return result;
}

TextMesh::TextMesh(const Graphics::Font& font, const std::wstring& text, std::optional<float> maxWidth, float fontSize, Align align)
{
	topology = skygfx::Topology::TriangleList;

	auto lines = CreateLines(font, text, maxWidth, fontSize);

	const auto texture = font.getTexture();
	glm::vec2 tex_size = { static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()) };

	size_t length = 0;

	for (const auto& line : lines)
	{
		length += std::distance(line.begin, line.end);
		size.x = glm::max(size.x, line.width);
	}

	if (maxWidth.has_value())
		size.x = glm::max(size.x, maxWidth.value() / font.getScaleFactorForSize(fontSize));

	vertices.resize(length * 4);
	indices.resize(length * 6);

	int i = 0;

	for (const auto& line : lines)
	{
		float pos_x = (size.x - line.width) * GetAlignNormalizedValue(align);

		for (auto it = line.begin; it != line.end; ++it, i++)
		{
			const auto& glyph = font.getGlyph(*it);

			auto vtx = &vertices[i * 4];
			auto idx = &indices[i * 6];

			pos_x += glyph.offset.x;
			float pos_y = size.y + font.getAscent() + glyph.offset.y;

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

			symbols.push_back(TextMesh::Symbol(p1, glyph.size, size.y));
		}

		size.y += font.getAscent() - font.getDescent() + font.getLinegap();
	}

	size.y -= font.getLinegap();
	size *= font.getScaleFactorForSize(fontSize);
}

void TextMesh::setSymbolColor(size_t index, const glm::vec4& color)
{
	size_t base_vtx = index * 4;

	for (size_t i = base_vtx; i < base_vtx + 4; i++)
	{
		vertices[i].color = color;
	}
}
