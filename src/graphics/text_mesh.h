#pragma once

#include <cstdint>
#include <renderer/all.h>
#include "font.h"
#include "tinyutf8.hpp"

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

		skygfx::Topology topology = skygfx::Topology::TriangleList;
		std::vector<skygfx::Vertex::PositionColorTexture> vertices;
		std::vector<uint32_t> indices;

		std::vector<glm::vec2> symbol_positions;
		std::vector<glm::vec2> symbol_sizes;
		std::vector<float> symbol_line_y;
	
		static TextMesh createTextMesh(const Font& font, utf8_string::iterator begin, utf8_string::iterator end);

		static TextMesh createSinglelineTextMesh(const Font& font, const utf8_string& text, float vertical_offset = 0.0f);

		static std::tuple<float, TextMesh> createMultilineTextMesh(const Font& font, const utf8_string& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}