#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <graphics/font.h>

namespace Scene
{
	class Label : public Node, public Color
	{
	public:
		inline static std::shared_ptr<Graphics::Font> DefaultFont = nullptr;
		inline static float DefaultFontSize = 24.0f;

	protected:
		void draw() override;
		void update(Clock::Duration dTime) override;

	public:
		void refresh();

	public:
		auto getFont() const { return mFont; }
		void setFont(const std::shared_ptr<Graphics::Font>& value) { mFont = value; }

		auto getFontSize() const { return mFontSize; }
		void setFontSize(float value) { mFontSize = value; }

		const auto& getText() const { return mText; }
		void setText(const utf8_string& value) { mText = value; }

		auto getOutlineThickness() const { return mOutlineThickness; }
		void setOutlineThickness(float value) { mOutlineThickness = value; }

		auto getOutlineColor() const { return mOutlineColor; }
		void setOutlineColor(const glm::vec4& value) { mOutlineColor = value; }
		void setOutlineColor(const glm::vec3& value) { mOutlineColor = { value, mOutlineColor.a }; }

		auto getOutlineAlpha() const { return mOutlineColor.a; }
		void setOutlineAlpha(float value) { mOutlineColor.a = value; }

		auto getAlign() const { return mAlign; }
		void setAlign(Graphics::TextMesh::Align value) { mAlign = value; }

		// returning [pos, size]
		std::tuple<glm::vec2, glm::vec2> getSymbolBounds(int index);
		float getSymbolLineY(int index);

	private:
		std::shared_ptr<Graphics::Font> mFont = DefaultFont;
		float mFontSize = DefaultFontSize;
		utf8_string mText = "";
		Graphics::TextMesh::Align mAlign = Graphics::TextMesh::Align::Left;
		Graphics::TextMesh mMesh;
		utf8_string mPrevText;
		float mPrevWidth = 0.0f;
		float mPrevFontSize = 0.0f;
		std::shared_ptr<Graphics::Font> mPrevFont;
		float mOutlineThickness = 0.0f;
		glm::vec4 mOutlineColor = { Graphics::Color::Black, 1.0f };
		Graphics::TextMesh::Align mPrevAlign = Graphics::TextMesh::Align::Left;
	};
}
