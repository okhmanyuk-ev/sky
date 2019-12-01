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

	private:
		std::shared_ptr<Graphics::Font> mFont;
		float mFontSize = 24.0f;
		utf8_string mText = "";
		bool mMultiline = false;
		Graphics::System::TextMesh mMesh;
		bool mMeshDirty = true;
		utf8_string mPrevText;
		float mPrevWidth = 0.0f;
		float mPrevFontSize = 0.0f;
		std::shared_ptr<Graphics::Font> mPrevFont;
		bool mPrevMultiline = false;
		glm::vec4 mPrevColor;
	};
}
