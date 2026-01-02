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

		TextMesh(const Graphics::Font& font, const std::wstring& text, std::optional<float> maxWidth, float fontSize, Align align);

		skygfx::Topology topology;
		Vertices vertices;
		Indices indices;
		Symbols symbols;
		glm::vec2 size = { 0.0f, 0.0f };

		void setSymbolColor(size_t index, const glm::vec4& color);
	};
}
