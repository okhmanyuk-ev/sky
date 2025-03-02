#include "label.h"
#include <regex>
#include <sky/utils.h>
#include <magic_enum/magic_enum.hpp>

using namespace Scene;

void Label::draw()
{
	Node::draw();

	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	if (getAlpha() <= 0.0f && (mOutlineColor->getAlpha() <= 0.0f || mOutlineThickness <= 0.0f))
		return;

	if (!mTextMesh.has_value())
		return;

	if (mTextMesh.value().vertices.empty())
		return;

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->pushSampler(skygfx::Sampler::Linear);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawString(*mFont, mTextMesh.value(), mFontSize, getColor(), mOutlineThickness, mOutlineColor->getColor());
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

	auto markDirtyIfChanged = [&](auto& prev, const auto& current) {
		if (prev != current)
		{
			prev = current;
			dirty = true;
		}
	};

	if (mMultiline)
		markDirtyIfChanged(mPrevWidth, width);

	markDirtyIfChanged(mPrevText, mText);
	markDirtyIfChanged(mPrevFontSize, mFontSize);
	markDirtyIfChanged(mPrevFont, mFont);
	markDirtyIfChanged(mPrevAlign, mAlign);
	markDirtyIfChanged(mPrevMultiline, mMultiline);
	markDirtyIfChanged(mPrevReplaceEscapedNewLines, mReplaceEscapedNewLines);
	markDirtyIfChanged(mPrevParseColorTags, mParseColorTags);

	if (!dirty)
		return;

	float height = 0.0f;

	auto text = mText;

	if (mReplaceEscapedNewLines)
		text = replaceEscapedNewlines(text);

	std::vector<glm::vec4> colormap;

	if (mParseColorTags)
		std::tie(colormap, text) = parseColorTags(text);

	if (!mMultiline)
	{
		height = (mFont->getAscent() - (mFont->getDescent() * 2.0f)) * mFont->getScaleFactorForSize(mFontSize);
		mTextMesh = Graphics::TextMesh::createSinglelineTextMesh(*mFont, text, -mFont->getDescent() + mFont->getCustomVerticalOffset());
		setWidth(mFont->getStringWidth(text, mFontSize));
	}
	else
	{
		std::tie(height, mTextMesh) = Graphics::TextMesh::createMultilineTextMesh(*mFont, text, width, mFontSize, mAlign);
	}

	for (size_t i = 0; i < colormap.size(); i++)
	{
		mTextMesh.value().setSymbolColor(i, colormap.at(i));
	}

	setHeight(height);

	updateAbsoluteSize();
}

std::tuple<glm::vec2, glm::vec2> Label::getSymbolBounds(int index)
{
	auto scale = mFont->getScaleFactorForSize(mFontSize);
	const auto& symbol = mTextMesh.value().symbols.at(index);
	auto pos = symbol.pos * scale;
	auto size = symbol.size * scale;
	return { pos, size };
}

float Label::getSymbolLineY(int index)
{
	auto scale = mFont->getScaleFactorForSize(mFontSize);
	return mTextMesh.value().symbols.at(index).line_y * scale;
}

std::tuple<std::vector<glm::vec4>, std::wstring> Label::parseColorTags(std::wstring str)
{
	auto parse_u8_color = [](auto match) {
		auto r = static_cast<uint8_t>(std::stoi(match[1]));
		auto g = static_cast<uint8_t>(std::stoi(match[2]));
		auto b = static_cast<uint8_t>(std::stoi(match[3]));
		auto a = match.size() > 4 ? static_cast<uint8_t>(std::stoi(match[4])) : 255;
		return sky::ColorToNormalized(r, g, b, a);
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
		return sky::ColorToNormalized(r, g, b, a);
	};

	auto parse_named_color = [](auto match) {
		auto str = sky::to_string(match[1].str());
		auto type = magic_enum::enum_cast<sky::Color>(str, magic_enum::case_insensitive);

		if (!type.has_value())
			return sky::GetColor<glm::vec4>(sky::Color::White);

		return sky::GetColor<glm::vec4>(type.value());
	};

	auto wregex = [](std::wstring expression) {
		return std::wregex(expression, std::regex_constants::ECMAScript | std::regex_constants::icase);
	};

	std::vector<std::tuple<std::wregex, std::function<glm::vec4(std::wsmatch match)>>> color_tags = {
		{ wregex(LR"(^<color=rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)>)"), parse_u8_color },
		{ wregex(LR"(^<color=rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)>)"), parse_u8_color },
		{ wregex(LR"(^<color=frgba\(\s*(\d+|\d+\.\d+)\s*,\s*(\d+|\d+\.\d+)\s*,\s*(\d+|\d+\.\d+)\s*,\s*(\d+|\d+\.\d+)\s*\)>)"), parse_float_color },
		{ wregex(LR"(^<color=frgb\(\s*(\d+|\d+\.\d+)\s*,\s*(\d+|\d+\.\d+)\s*,\s*(\d+|\d+\.\d+)\s*\)>)"), parse_float_color },
		{ wregex(LR"(^<color=hex\(\s*([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})\s*\)>)"), parse_hex_color },
		{ wregex(LR"(^<color=\s*([A-Za-z]+)\s*>)"), parse_named_color },
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

	auto pop_color_regex = wregex(LR"(^</color>)");
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
}

std::wstring Label::replaceEscapedNewlines(const std::wstring& str)
{
	std::wregex pattern(LR"(\\n)");
	return std::regex_replace(str, pattern, L"\n");
}
