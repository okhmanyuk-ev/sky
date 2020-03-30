#pragma once

#include <cstdint>
#include <renderer/vertex.h>
#include <renderer/topology.h>
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

		Renderer::Topology topology;
		std::vector<Renderer::Vertex::PositionColorTexture> vertices;
		std::vector<uint32_t> indices;
	
		static TextMesh createTextMesh(const Font& font, utf8_string::iterator begin, utf8_string::iterator end);

		static TextMesh createSinglelineTextMesh(const Font& font, const utf8_string& text, float vertical_offset = 0.0f);

		static std::tuple<float, TextMesh> createMultilineTextMesh(const Font& font, const utf8_string& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}