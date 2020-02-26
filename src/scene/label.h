#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Graphics/font.h>

namespace Scene
{
	class Label : public Node, public Color
	{
	protected:
		void draw() override;
		void update() override;

	public:
		auto getFont() const { return mFont; }
		void setFont(const std::shared_ptr<Graphics::Font>& value) { mFont = value; }

		auto getFontSize() const { return mFontSize; }
		void setFontSize(float value) { mFontSize = value; }

		const auto& getText() const { return mText; }
		void setText(const utf8_string& value) { mText = value; }

		auto isMultiline() const { return mMultiline; }
		void setMultiline(bool value) { mMultiline = value; }

		auto getOutlineThickness() const { return mOutlineThickness; }
		void setOutlineThickness(float value) { mOutlineThickness = value; }

		auto getOutlineColor() const { return mOutlineColor; }
		void setOutlineColor(const glm::vec4& value) { mOutlineColor = value; }
		void setOutlineColor(const glm::vec3& value) { mOutlineColor = { value, mOutlineColor.a }; }

		auto getOutlineAlpha() const { return mOutlineColor.a; }
		void setOutlineAlpha(float value) { mOutlineColor.a = value; }

		auto getMultilineAlign() const { return mMultilineAlign; }
		void setMultilineAlign(Graphics::TextAlign value) { mMultilineAlign = value; }

	private:
		std::shared_ptr<Graphics::Font> mFont;
		float mFontSize = 24.0f;
		utf8_string mText = "";
		bool mMultiline = false;
		Graphics::TextAlign mMultilineAlign = Graphics::TextAlign::Left;
		Graphics::Mesh mMesh;
		float mMeshWidth = 0.0f;
		float mMeshHeight = 0.0f;
		bool mMeshDirty = true;
		utf8_string mPrevText;
		float mPrevWidth = 0.0f;
		float mPrevFontSize = 0.0f;
		std::shared_ptr<Graphics::Font> mPrevFont;
		bool mPrevMultiline = false;
		float mOutlineThickness = 0.0f;
		glm::vec4 mOutlineColor = { Graphics::Color::Black, 1.0f };
		Graphics::TextAlign mPrevMultilineAlign = Graphics::TextAlign::Left;
	};
}
