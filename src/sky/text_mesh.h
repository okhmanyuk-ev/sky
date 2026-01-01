#pragma once

#include <cstdint>
#include <graphics/font.h>
#include <skygfx/utils.h>

namespace sky
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

		TextMesh(skygfx::Topology topology, Vertices vertices, Indices indices, Symbols symbols, float height);

		skygfx::Topology topology;
		Vertices vertices;
		Indices indices;
		Symbols symbols;
		float height;

		void setSymbolColor(size_t index, const glm::vec4& color);

		static TextMesh createTextMesh(const Graphics::Font & font, std::wstring::const_iterator begin,
			std::wstring::const_iterator end, float size);

		static TextMesh createTextMesh(const Graphics::Font& font, const std::wstring& text, float size);

		static TextMesh createMultilineTextMesh(const Graphics::Font& font, const std::wstring& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}
