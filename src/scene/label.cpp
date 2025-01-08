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

void Label::update(sky::Duration dTime)
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

	auto replace_escaped_newlines = [](const std::wstring& input) {
		std::wregex pattern(LR"(\\n)");
		return std::regex_replace(input, pattern, L"\n");
	};

	auto parse_color_tags = [](std::wstring str) {
		auto parse_u8_color = [](auto match) {
			auto r = static_cast<uint8_t>(std::stoi(match[1]));
			auto g = static_cast<uint8_t>(std::stoi(match[2]));
			auto b = static_cast<uint8_t>(std::stoi(match[3]));
			auto a = match.size() > 4 ? static_cast<uint8_t>(std::stoi(match[4])) : 255;
			return Graphics::Color::ToNormalized(r, g, b, a);
		};

		auto parse_float_color = [](auto match) {
			auto r = std::stof(match[1]);
			auto g = std::stof(match[2]);
			auto b = std::stof(match[3]);
			auto a = match.size() > 4 ? std::stof(match[4]) : 1.0f;
			return glm::vec4{ r, g, b, a };
		};

		auto parse_hex_color = [](auto match) {
			auto hex_color = match[1];
			auto rgba = std::stoul(hex_color, nullptr, 16);
			auto has_alpha = hex_color.length() == 8;
			auto r = static_cast<uint8_t>((rgba >> (has_alpha ? 24 : 16)) & 0xFF);
			auto g = static_cast<uint8_t>((rgba >> (has_alpha ? 16 : 8)) & 0xFF);
			auto b = static_cast<uint8_t>((rgba >> (has_alpha ? 8 : 0)) & 0xFF);
			auto a = static_cast<uint8_t>(has_alpha ? (rgba & 0xFF) : 255);
			return Graphics::Color::ToNormalized(r, g, b, a);
		};

		std::vector<std::tuple<std::wregex, std::function<glm::vec4(std::wsmatch match)>>> color_tags = {
			{ std::wregex(LR"(^<color=rgba\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\)>)"), parse_u8_color },
			{ std::wregex(LR"(^<color=rgb\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\)>)"), parse_u8_color },
			{ std::wregex(LR"(^<color=frgba\([ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*\)>)"), parse_float_color },
			{ std::wregex(LR"(^<color=frgb\([ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*\)>)"), parse_float_color },
			{ std::wregex(LR"(^<color=hex\([ ]*([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})[ ]*\)>)"), parse_hex_color },
		};

		std::wsmatch match;

		std::stack<glm::vec4> color_stack;
		color_stack.push({ 1.0f, 1.0f, 1.0f, 1.0f });

		auto parse_color_push_tag = [&] {
			for (const auto& [regex, callback] : color_tags)
			{
				if (std::regex_search(str, match, regex))
				{
					color_stack.push(callback(match));
					str.erase(0, match.length());
					return true;
				}
			}
			return false;
		};

		std::wregex pop_color_regex(LR"(^</color>)");
		std::wstring result_text;
		std::vector<glm::vec4> colormap;

		while (!str.empty())
		{
			if (parse_color_push_tag())
				continue;

			if (std::regex_search(str, match, pop_color_regex))
			{
				color_stack.pop();
				str.erase(0, match.length());
			}
			else
			{
				result_text.push_back(str.front());
				str.erase(0, 1);
				colormap.push_back(color_stack.top());
			}
		}

		return std::make_tuple(colormap, result_text);
	};

	auto text = mText;

	if (mReplaceEscapedNewLines)
		text = replace_escaped_newlines(text);

	std::vector<glm::vec4> colormap;

	if (mParseColorTags)
		std::tie(colormap, text) = parse_color_tags(text);

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
