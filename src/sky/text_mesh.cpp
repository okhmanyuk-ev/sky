#include "text_mesh.h"
#include <sky/color.h>

using namespace sky;

TextMesh::Symbol::Symbol(glm::vec2 pos, glm::vec2 size, float line_y) :
	pos(pos), size(size), line_y(line_y)
{
}

TextMesh::TextMesh(skygfx::Topology topology, Vertices vertices, Indices indices, Symbols symbols) :
	topology(topology),
	vertices(std::move(vertices)),
	indices(std::move(indices)),
	symbols(std::move(symbols))
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

TextMesh TextMesh::createTextMesh(const Graphics::Font& font, std::wstring::const_iterator begin,
	std::wstring::const_iterator end)
{
	const auto texture = font.getTexture();

	glm::vec2 tex_size = { static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()) };

	auto length = std::distance(begin, end);

	auto vertices = Vertices(length * 4);
	auto indices = Indices(length * 6);
	Symbols symbols;

	glm::vec2 pos = { 0.0f, 0.0f };

	int i = 0;

	for (auto it = begin; it != end; ++it, i++)
	{
		const auto& glyph = font.getGlyph(*it);

		auto vtx = &vertices[i * 4];
		auto idx = &indices[i * 6];

		pos.x += glyph.offset.x;
		pos.y = font.getAscent() + glyph.offset.y;

		auto p1 = pos;
		auto p2 = pos + glyph.size;

		pos.x -= glyph.offset.x;
		pos.x += glyph.xadvance;

		if (it != end - 1)
		{
			pos.x += font.getKerning(*it, *(it + 1));
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

		symbols.push_back(Symbol(p1, glyph.size, 0.0f));
	}

	return TextMesh(skygfx::Topology::TriangleList, std::move(vertices), std::move(indices), std::move(symbols));
}

TextMesh TextMesh::createSinglelineTextMesh(const Graphics::Font& font, const std::wstring& text)
{
	return createTextMesh(font, text.begin(), text.end());
}

static float GetAlignNormalizedValue(TextMesh::Align align)
{
	if (align == TextMesh::Align::Center)
		return 0.5f;

	if (align == TextMesh::Align::Right)
		return 1.0f;

	return 0.0f;
}

std::tuple<float, TextMesh> TextMesh::createMultilineTextMesh(const Graphics::Font& font, const std::wstring& text,
	float maxWidth, float size, Align align)
{
	auto scale = font.getScaleFactorForSize(size);
	float height = 0.0f;

	float scaledMaxWidth = maxWidth / scale;

	Vertices vertices;
	Indices indices;
	Symbols symbols;

	auto appendTextMesh = [&](std::wstring::const_iterator begin, std::wstring::const_iterator end) {
		auto mesh = createTextMesh(font, begin, end);
		for (auto index : mesh.indices)
		{
			index += static_cast<uint32_t>(vertices.size());
			indices.push_back(index);
		}
		auto str_w = font.getStringWidth(begin, end);
		for (auto vertex : mesh.vertices)
		{
			vertex.pos.x += (scaledMaxWidth - str_w) * GetAlignNormalizedValue(align);
			vertex.pos.y += height;
			vertices.push_back(vertex);
		}
		for (const auto& symbol_info : mesh.symbols)
		{
			auto pos = symbol_info.pos;
			pos.x += (scaledMaxWidth - str_w) * GetAlignNormalizedValue(align);
			pos.y += height;
			auto line_y = symbol_info.line_y + height;
			symbols.push_back(Symbol(pos, symbol_info.size, line_y));
		}
		height += font.getAscent() - font.getDescent() + font.getLinegap();
	};

	auto begin = text.begin();
	auto it = begin;

	while (it != text.end())
	{
		std::advance(it, 1);

		if (it != text.end() && *it == '\n')
		{
			appendTextMesh(begin, it);
			begin = it + 1;
			continue;
		}

		auto length = std::distance(begin, it);

		if (length <= 1)
			continue;

		auto str_w = font.getStringWidth(begin, it);

		if (str_w <= scaledMaxWidth)
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

		appendTextMesh(begin, it);
		begin = it;
	}

	if (begin != text.end())
		appendTextMesh(begin, text.end());

	height -= font.getLinegap();

	auto mesh = TextMesh(skygfx::Topology::TriangleList, std::move(vertices), std::move(indices), std::move(symbols));

	return { height * scale, mesh };
}
