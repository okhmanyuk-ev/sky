#pragma once

#include <scene/node.h>
#include <scene/auto_sized.h>
#include <scene/label.h>
#include <scene/grid.h>
#include <graphics/tex_cell.h>

namespace Scene
{
	class RichLabel : public AutoSized<Node>
	{
	public:
		using IconTextureCallback = std::function<Graphics::TexCell(const std::string& path)>;
		inline static IconTextureCallback DefaultIconTextureCallback = nullptr;

	public:
		RichLabel();

	protected:
		void update(Clock::Duration dTime) override;

	public:
		void refresh();

	public:
		auto getFont() const { return mState.font; }
		void setFont(const std::shared_ptr<Graphics::Font>& value) { mState.font = value; }

		auto getFontSize() const { return mState.font_size; }
		void setFontSize(float value) { mState.font_size = value; }

		const auto& getText() const { return mState.text; }
		void setText(std::wstring text) { mState.text = std::move(text); }

		void setTag(const std::string& name, std::function<std::shared_ptr<Node>()> callback);

	private:
		struct State
		{
			std::wstring text;
			std::shared_ptr<Graphics::Font> font = Label::DefaultFont;
			float font_size = Label::DefaultFontSize;
			bool operator==(const State& other) const = default;
		};
		State mState;
		State mPrevState;
		std::shared_ptr<AutoSized<Row>> mContent;
		std::unordered_map<std::string, std::function<std::shared_ptr<Node>()>> mTags;
	};
}
