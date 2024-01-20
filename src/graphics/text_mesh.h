#pragma once

#include <cstdint>
#include <renderer/all.h>
#include <tinyutf8/tinyutf8.h>
#include "font.h"

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
		skygfx::utils::Mesh::Vertices vertices;
		skygfx::utils::Mesh::Indices indices;

		std::vector<glm::vec2> symbol_positions;
		std::vector<glm::vec2> symbol_sizes;
		std::vector<float> symbol_line_y;
	
		static TextMesh createTextMesh(const Font& font, tiny_utf8::string::const_iterator begin,
			tiny_utf8::string::const_iterator end);

		static TextMesh createSinglelineTextMesh(const Font& font, const tiny_utf8::string& text,
			float vertical_offset = 0.0f);

		static std::tuple<float, TextMesh> createMultilineTextMesh(const Font& font, const tiny_utf8::string& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}
