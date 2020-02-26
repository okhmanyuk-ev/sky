#pragma once

#include <cstdint>
#include <Renderer/vertex.h>
#include <Renderer/topology.h>
#include "font.h"
#include "tinyutf8.hpp"
#include "mesh.h"
#include "color.h"

namespace Graphics
{
	enum class TextAlign
	{
		Left,
		Center,
		Right
	};
	
	Mesh CreateTextMesh(const Font& font, utf8_string::iterator begin, utf8_string::iterator end);

	Mesh CreateSinglelineTextMesh(const Font& font, const utf8_string& text, float vertical_offset = 0.0f);

	std::tuple<float, Mesh> CreateMultilineTextMesh(const Font& font, const utf8_string& text,
		float maxWidth, float size, TextAlign align = TextAlign::Left);
}