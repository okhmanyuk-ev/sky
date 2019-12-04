#include "text_mesh.h"

using namespace Graphics;

TextMesh TextMesh::createTextMesh(const Font& font, utf8_string::iterator begin, utf8_string::iterator end, float height)
{
	TextMesh mesh;

	mesh.topology = Renderer::Topology::TriangleList;

	const auto texture = font.getTexture();

	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	auto length = std::distance(begin, end);
	mesh.vertices.resize(length * 4);
	mesh.indices.resize(length * 6);

	glm::vec2 pos = { 0.0f, 0.0f };
	int i = 0;

	for (auto it = begin; it != end; ++it, i++)
	{
		const auto& glyph = font.getGlyph(*it);

		float glyph_w = static_cast<float>(glyph.w);
		float glyph_h = static_cast<float>(glyph.h);
		float glyph_x = static_cast<float>(glyph.x);
		float glyph_y = static_cast<float>(glyph.y);

		auto vtx = &mesh.vertices[i * 4];
		auto idx = &mesh.indices[i * 6];

		pos.y = glyph.yoff;
		pos.x += glyph.xoff;

		float x1 = pos.x;
		float x2 = pos.x + glyph_w;
		float y1 = pos.y + height;
		float y2 = pos.y + height + glyph_h;

		pos.x -= glyph.xoff;
		pos.x += glyph.xadvance;

		if (it != end - 1)
		{
			pos.x += font.getKerning(*it, *(it + 1));
		}

		float u1 = glyph_x / tex_w;
		float v1 = glyph_y / tex_h;
		float u2 = (glyph_x + glyph_w) / tex_w;
		float v2 = (glyph_y + glyph_h) / tex_h;

		vtx[0] = { { x1, y1, 0.0f }, { u1, v1 } };
		vtx[1] = { { x1, y2, 0.0f }, { u1, v2 } };
		vtx[2] = { { x2, y2, 0.0f }, { u2, v2 } };
		vtx[3] = { { x2, y1, 0.0f }, { u2, v1 } };

		auto base_vtx = i * 4;

		idx[0] = base_vtx + 0;
		idx[1] = base_vtx + 1;
		idx[2] = base_vtx + 2;
		idx[3] = base_vtx + 0;
		idx[4] = base_vtx + 2;
		idx[5] = base_vtx + 3;
	}

	return mesh;
}

TextMesh TextMesh::createSinglelineTextMesh(const Font& font, const utf8_string& text)
{
	return createTextMesh(font, text.begin(), text.end(), font.getStringHeight(text));
}

std::tuple<float, TextMesh> TextMesh::createMultilineTextMesh(const Font& font, const utf8_string& text,
	float maxWidth, float size, Align align)
{
	auto scale = font.getScaleFactorForSize(size);
	float height = 0.0f;

	TextMesh result;

	bool firstLine = true;
	auto getStringHeight = [&firstLine, scale, size, &font](utf8_string::iterator begin, utf8_string::iterator end) {
		if (firstLine)
		{
			firstLine = false;
			return font.getStringHeight(begin, end);
		}
		return size / scale;
	};

	auto appendTextMesh = [&font, scale, &height, getStringHeight, &result, align, maxWidth](utf8_string::iterator begin, utf8_string::iterator end, bool lastLine = false) {
		auto str_h = getStringHeight(begin, end);
		auto str_w = font.getStringWidth(begin, end);
		auto mesh = createTextMesh(font, begin, end, str_h);
		for (auto index : mesh.indices)
		{
			index += result.vertices.size();
			result.indices.push_back(index);
		}
		for (auto vertex : mesh.vertices)
		{
			if (align == Align::Right)
				vertex.pos.x += (maxWidth / scale) - str_w;
			else if (align == Align::Center)
				vertex.pos.x += ((maxWidth / scale) - str_w) / 2.0f;

			vertex.pos.y += height;
			result.vertices.push_back(vertex);
		}
		if (lastLine)
			height += font.getStringHeight(begin, end) / scale;
		else
			height += str_h;
	};

	result.topology = Renderer::Topology::TriangleList;

	auto begin = text.begin();
	auto it = begin;

	while (it != text.end())
	{
		auto str_w = font.getStringWidth(begin, it, size);

		if (str_w >= maxWidth)
		{
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

		++it;
	}

	if (begin != text.end())
		appendTextMesh(begin, text.end(), true);

	return { height * scale, result };
}