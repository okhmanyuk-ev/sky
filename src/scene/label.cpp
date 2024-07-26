#include "label.h"
#include <regex>

using namespace Scene;

void Label::draw()
{
	Node::draw();

	assert(mFont); // you should setup Label::DefaultFont

	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	if (getAlpha() <= 0.0f && (mOutlineColor->getAlpha() <= 0.0f || mOutlineThickness <= 0.0f))
		return;

	if (mMesh.vertices.empty())
		return;

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	auto absoluteScale = getAbsoluteScale();
	auto smoothFactorScale = 1.0f / ((absoluteScale.x + absoluteScale.y) * 0.5f);

	GRAPHICS->pushSampler(skygfx::Sampler::Linear);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawString(*mFont, mMesh, mFontSize, getColor(), mOutlineThickness, mOutlineColor->getColor(),
		smoothFactorScale);
	GRAPHICS->pop(2);
}

void Label::update(Clock::Duration dTime)
{
	Node::update(dTime);
	refresh();
}

void Label::refresh()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	auto dirty = false;
	auto width = getAbsoluteWidth();

	if (mPrevWidth != width && mMultiline)
	{
		mPrevWidth = width;
		dirty = true;
	}

	if (mPrevText != mText)
	{
		mPrevText = mText;
		dirty = true;
	}

	if (mPrevFontSize != mFontSize)
	{
		mPrevFontSize = mFontSize;
		dirty = true;
	}

	if (mPrevFont != mFont)
	{
		mPrevFont = mFont;
		dirty = true;
	}

	if (mPrevAlign != mAlign)
	{
		mPrevAlign = mAlign;
		dirty = true;
	}

	if (mPrevMultiline != mMultiline)
	{
		mPrevMultiline = mMultiline;
		dirty = true;
	}

	if (mPrevReplaceEscapedNewLines != mReplaceEscapedNewLines)
	{
		mPrevReplaceEscapedNewLines = mReplaceEscapedNewLines;
		dirty = true;
	}

	if (mPrevParseColorTags != mParseColorTags)
	{
		mPrevParseColorTags = mParseColorTags;
		dirty = true;
	}

	if (!dirty)
		return;

	float height = 0.0f;

	auto replaceEscapedNewlines = [](const std::wstring& input) {
		std::wregex pattern(LR"(\\n)");
		return std::regex_replace(input, pattern, L"\n");
	};

	auto parseColorTags = [](std::wstring str) {
		std::vector<glm::vec4> colormap;
		std::wstring sublimed_text;

		std::wregex color_open_rgba_tag(LR"(^<color=rgba\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\)>)");
		std::wregex color_open_rgb_tag(LR"(^<color=rgb\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\)>)");
		std::wregex color_open_hex_rgba_tag(LR"(^<color=hex\([ ]*([0-9A-Fa-f]{8})[ ]*\)>)");
		std::wregex color_open_hex_rgb_tag(LR"(^<color=hex\([ ]*([0-9A-Fa-f]{6})[ ]*\)>)");
		std::wregex color_close_tag(LR"(^</color>)");
		std::wsmatch match;

		const glm::vec4 default_color = { 1.0f, 1.0f, 1.0f, 1.0f };

		glm::vec4 color = default_color;

		while (!str.empty())
		{
			if (std::regex_search(str, match, color_open_rgba_tag))
			{
				uint8_t r = std::stoi(match[1]);
				uint8_t g = std::stoi(match[2]);
				uint8_t b = std::stoi(match[3]);
				uint8_t a = std::stoi(match[4]);

				color = Graphics::Color::ToNormalized(r, g, b, a);
				str.erase(0, match.length());
			}
			else if (std::regex_search(str, match, color_open_rgb_tag))
			{
				uint8_t r = std::stoi(match[1]);
				uint8_t g = std::stoi(match[2]);
				uint8_t b = std::stoi(match[3]);

				color = Graphics::Color::ToNormalized(r, g, b, 255);
				str.erase(0, match.length());
			}
			else if (std::regex_search(str, match, color_open_hex_rgba_tag))
			{
				auto hex_color = match[1];
				auto rgba = std::stoul(hex_color, nullptr, 16);
				color = Graphics::Color::ToNormalized((rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF,
					(rgba >> 8) & 0xFF, rgba & 0xFF);
				str.erase(0, match.length());
			}
			else if (std::regex_search(str, match, color_open_hex_rgb_tag))
			{
				auto hex_color = match[1];
				auto rgb = std::stoul(hex_color, nullptr, 16) << 8;
				color = Graphics::Color::ToNormalized((rgb >> 24) & 0xFF, (rgb >> 16) & 0xFF,
					(rgb >> 8) & 0xFF, 255);
				str.erase(0, match.length());
			}
			else if (std::regex_search(str, match, color_close_tag))
			{
				color = default_color;
				str.erase(0, match.length());
			}
			else
			{
				sublimed_text.push_back(str.front());
				str.erase(0, 1);
				colormap.push_back(color);
			}
		}

		return std::make_tuple(colormap, sublimed_text);
	};

	auto text = mText;

	if (mReplaceEscapedNewLines)
		text = replaceEscapedNewlines(text);

	std::vector<glm::vec4> colormap;

	if (mParseColorTags)
		std::tie(colormap, text) = parseColorTags(text);

	if (!mMultiline)
	{
		height = (mFont->getAscent() - (mFont->getDescent() * 2.0f)) * mFont->getScaleFactorForSize(mFontSize);
		mMesh = Graphics::TextMesh::createSinglelineTextMesh(*mFont, text, -mFont->getDescent() + mFont->getCustomVerticalOffset());
		setWidth(mFont->getStringWidth(text, mFontSize));
	}
	else
	{
		std::tie(height, mMesh) = Graphics::TextMesh::createMultilineTextMesh(*mFont, text, width, mFontSize, mAlign);
	}

	for (size_t i = 0; i < colormap.size(); i++)
	{
		setSymbolColor(i, colormap.at(i));
	}

	setHeight(height);

	updateAbsoluteSize();
}

std::tuple<glm::vec2, glm::vec2> Label::getSymbolBounds(int index)
{
	auto scale = mFont->getScaleFactorForSize(mFontSize);

	auto pos = mMesh.symbol_positions.at(index) * scale;
	auto size = mMesh.symbol_sizes.at(index) * scale;

	return { pos, size };
}

float Label::getSymbolLineY(int index)
{
	auto scale = mFont->getScaleFactorForSize(mFontSize);

	return mMesh.symbol_line_y.at(index) * scale;
}

void Label::setSymbolColor(size_t index, const glm::vec4& color)
{
	mMesh.setSymbolColor(index, color);
}
