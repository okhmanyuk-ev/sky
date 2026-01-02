#pragma once

#include <cstdint>
#include <graphics/font.h>
#include <skygfx/utils.h>

namespace sky
{
	class TextMesh
	{
	public:
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

		using Symbols = std::vector<Symbol>;
		using Vertices = skygfx::utils::Mesh::Vertices;
		using Indices = skygfx::utils::Mesh::Indices;

		TextMesh(const Graphics::Font& font, const std::wstring& text, std::optional<float> maxWidth, float fontSize, Align align);

		void setSymbolColor(size_t index, const glm::vec4& color);

		auto getTopology() const { return mTopology; }
		const auto& getVertices() const { return mVertices; }
		const auto& getIndices() const { return mIndices; }
		const auto& getSymbols() const { return mSymbols; }
		const auto& getSize() const { return mSize; }

	private:
		skygfx::Topology mTopology;
		Indices mIndices;
		Vertices mVertices;
		Symbols mSymbols;
		glm::vec2 mSize = { 0.0f, 0.0f };
	};
}
