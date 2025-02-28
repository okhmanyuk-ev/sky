#pragma once

#include <cstdint>
#include "font.h"
#include <skygfx/utils.h>

namespace Graphics
{
	struct TextMesh
	{
		enum class Align
		{
			Left,
			Center,
			Right
		};

		struct Symbol
		{
			Symbol(glm::vec2 pos, glm::vec2 size, float line_y);
			glm::vec2 pos;
			glm::vec2 size;
			float line_y;
		};

		using Vertices = skygfx::utils::Mesh::Vertices;
		using Indices = skygfx::utils::Mesh::Indices;
		using Symbols = std::vector<Symbol>;

		TextMesh(skygfx::Topology topology, Vertices vertices, Indices indices, Symbols symbols);

		skygfx::Topology topology;
		Vertices vertices;
		Indices indices;
		Symbols symbols;

		void setSymbolColor(size_t index, const glm::vec4& color);

		static TextMesh createTextMesh(const Font& font, std::wstring::const_iterator begin,
			std::wstring::const_iterator end);

		static TextMesh createSinglelineTextMesh(const Font& font, const std::wstring& text,
			float vertical_offset = 0.0f);

		static std::tuple<float, TextMesh> createMultilineTextMesh(const Font& font, const std::wstring& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}
