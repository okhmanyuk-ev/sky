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
		void setText(const tiny_utf8::string& value) { mText = value; }

		auto getOutlineThickness() const { return mOutlineThickness; }
		void setOutlineThickness(float value) { mOutlineThickness = value; }

		auto getOutlineColor() const { return mOutlineColor; }

		auto getAlign() const { return mAlign; }
		void setAlign(Graphics::TextMesh::Align value) { mAlign = value; }

		auto isMultiline() const { return mMultiline; }
		void setMultiline(bool value) { mMultiline = value; }

		// returning [pos, size]
		std::tuple<glm::vec2, glm::vec2> getSymbolBounds(int index);
		float getSymbolLineY(int index);

		void setSymbolColor(size_t index, const glm::vec4& color);

		bool isReplaceEscapedNewLinesEnabled() const { return mReplaceEscapedNewLines; }
		void setReplaceEscapedNewLinesEnabled(bool value) { mReplaceEscapedNewLines = value; }

	private:
		std::shared_ptr<Graphics::Font> mFont = DefaultFont;
		float mFontSize = DefaultFontSize;
		tiny_utf8::string mText = "";
		Graphics::TextMesh::Align mAlign = Graphics::TextMesh::Align::Left;
		Graphics::TextMesh mMesh;
		tiny_utf8::string mPrevText;
		float mPrevWidth = 0.0f;
		float mPrevFontSize = 0.0f;
		std::shared_ptr<Graphics::Font> mPrevFont;
		float mOutlineThickness = 0.0f;
		std::shared_ptr<Color> mOutlineColor = std::make_shared<Color>(Graphics::Color::Black);
		Graphics::TextMesh::Align mPrevAlign = Graphics::TextMesh::Align::Left;
		bool mMultiline = false;
		bool mPrevMultiline = false;
		bool mReplaceEscapedNewLines = true;
		bool mPrevReplaceEscapedNewLines = true;
	};
}
