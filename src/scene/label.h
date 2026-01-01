#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <graphics/font.h>
#include <sky/text_mesh.h>

namespace Scene
{
	class Label : public Node, public Color
	{
	public:
		inline static std::shared_ptr<Graphics::Font> DefaultFont = nullptr;
		inline static float DefaultFontSize = 24.0f;

		enum class Bold
		{
			None,
			SemiBold,
			Bold,
			ExtraBold
		};

	protected:
		void draw() override;
		void update(sky::Duration dTime) override;

	public:
		void refresh();

	public:
		auto getFont() const { return mSettings.font; }
		void setFont(std::shared_ptr<Graphics::Font> value) { mSettings.font = value; }

		auto getFontSize() const { return mSettings.font_size; }
		void setFontSize(float value) { mSettings.font_size = value; }

		auto getBold() const { return mBold; }
		void setBold(Bold value) { mBold = value; }

		const auto& getText() const { return mSettings.text; }
		void setText(const std::wstring& value) { mSettings.text = value; }

		auto getOutlineThickness() const { return mOutlineThickness; }
		void setOutlineThickness(float value) { mOutlineThickness = value; }

		auto getOutlineColor() const { return mOutlineColor; }

		auto getAlign() const { return mSettings.align; }
		void setAlign(sky::TextMesh::Align value) { mSettings.align = value; }

		auto isWordWrapMode() const { return mSettings.word_wrap_mode; }
		void setWordWrapMode(bool value) { mSettings.word_wrap_mode = value; }

		// returning [pos, size]
		std::tuple<glm::vec2, glm::vec2> getSymbolBounds(int index);
		float getSymbolLineY(int index);

		bool isReplaceEscapedNewLinesEnabled() const { return mSettings.replace_escaped_new_lines; }
		void setReplaceEscapedNewLinesEnabled(bool value) { mSettings.replace_escaped_new_lines = value; }

		bool isParseColorTagsEnabled() const { return mSettings.parse_color_tags; }
		void setParseColorTagsEnabled(bool value) { mSettings.parse_color_tags = value; }

		bool isParseLocaleTagsEnabled() const { return mSettings.parse_locale_tags; }
		void setParseLocaleTagsEnabled(bool value) { mSettings.parse_locale_tags = value; }

		const auto& getTextMesh() const { return mTextMesh; }

	private:
		Bold mBold = Bold::None;
		std::optional<sky::TextMesh> mTextMesh;
		float mPrevWidth = 0.0f;
		float mOutlineThickness = 0.0f;
		std::shared_ptr<Color> mOutlineColor = std::make_shared<Color>(sky::GetColor(sky::Color::Black));

		struct Settings
		{
			std::wstring text;
			std::shared_ptr<Graphics::Font> font = DefaultFont;
			float font_size = DefaultFontSize;
			sky::TextMesh::Align align = sky::TextMesh::Align::Left;
			bool word_wrap_mode = false;
			bool replace_escaped_new_lines = false;
			bool parse_color_tags = false;
			bool parse_locale_tags = true;

			bool operator==(const Settings& other) const = default;
		};

		Settings mSettings;
		std::optional<Settings> mPrevSettings;
	};
}
