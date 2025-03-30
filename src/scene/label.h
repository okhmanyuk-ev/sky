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
		void update(sky::Duration dTime) override;

	public:
		void refresh();

	public:
		auto getFont() const { return mFont; }
		void setFont(std::shared_ptr<Graphics::Font> value) { mFont = value; }

		auto getFontSize() const { return mFontSize; }
		void setFontSize(float value) { mFontSize = value; }

		const auto& getText() const { return mText; }
		void setText(const std::wstring& value) { mText = value; }

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

		bool isReplaceEscapedNewLinesEnabled() const { return mReplaceEscapedNewLines; }
		void setReplaceEscapedNewLinesEnabled(bool value) { mReplaceEscapedNewLines = value; }

		bool isParseColorTagsEnabled() const { return mParseColorTags; }
		void setParseColorTagsEnabled(bool value) { mParseColorTags = value; }

		bool isParseLocaleTagsEnabled() const { return mParseLocaleTags; }
		void setParseLocaleTagsEnabled(bool value) { mParseLocaleTags = value; }

		const auto& getTextMesh() const { return mTextMesh; }

	private:
		std::tuple<std::vector<glm::vec4>, std::wstring> parseColorTags(std::wstring str);
		std::wstring replaceEscapedNewlines(const std::wstring& str);

	private:
		std::shared_ptr<Graphics::Font> mFont = DefaultFont;
		float mFontSize = DefaultFontSize;
		std::wstring mText;
		Graphics::TextMesh::Align mAlign = Graphics::TextMesh::Align::Left;
		std::optional<Graphics::TextMesh> mTextMesh;
		std::wstring mPrevText;
		float mPrevWidth = 0.0f;
		float mPrevFontSize = 0.0f;
		std::shared_ptr<Graphics::Font> mPrevFont;
		float mOutlineThickness = 0.0f;
		std::shared_ptr<Color> mOutlineColor = std::make_shared<Color>(sky::GetColor(sky::Color::Black));
		Graphics::TextMesh::Align mPrevAlign = Graphics::TextMesh::Align::Left;
		bool mMultiline = false;
		bool mPrevMultiline = false;
		bool mReplaceEscapedNewLines = false;
		bool mPrevReplaceEscapedNewLines = false;
		bool mParseColorTags = false;
		bool mPrevParseColorTags = false;
		bool mParseLocaleTags = true;
		bool mPrevParseLocaleTags = true;
	};
}
