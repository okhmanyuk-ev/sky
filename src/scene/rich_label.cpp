#include "rich_label.h"
#include <scene/adaptive.h>
#include <scene/sprite.h>
#include <regex>
#include <common/helpers.h>
#include <sky/utils.h>

using namespace Scene;

RichLabel::RichLabel()
{
	mContent = std::make_shared<AutoSized<Row>>();
	mContent->setAnchor(0.5f);
	mContent->setPivot(0.5f);
	attach(mContent);
}

void RichLabel::update(sky::Duration dTime)
{
	AutoSized<Node>::update(dTime);
	refresh();
}

void RichLabel::refresh()
{
	if (mState == mPrevState)
		return;

	mPrevState = mState;
	mContent->clear();

	auto append = [&](std::shared_ptr<Node> item, bool stretch) {
		auto holder = std::make_shared<AutoSized<Node>>();
		holder->setAnchor({ 0.0f, 0.5f });
		holder->setPivot({ 0.0f, 0.5f });
		holder->setAutoHeightEnabled(!stretch);
		holder->setVerticalStretch(stretch ? 1.0f : 0.0f);
		holder->attach(item);
		mContent->attach(holder);
	};

	auto createLabel = [&](const std::wstring& str) {
		auto label = std::make_shared<Label>();
		label->setText(str);
		label->setParseColorTagsEnabled(true);
		label->setFont(mState.font);
		label->setFontSize(mState.font_size);
		return label;
	};

	auto createSprite = [this_index = size_t(this)](const std::string& path) {
		auto sprite = std::make_shared<Adaptive<Sprite>>();
		sprite->setAdaptBehavior(AdaptBehavior::Height);
		sprite->setTexture(sky::GetTexture(path));
		sprite->setBatchGroup(std::format("rich_label_icon_{}_{}", this_index, path));
		return sprite;
	};

	std::wregex icon_tag(LR"(^<icon=([^>]+)>)");

	std::wstring sublimed_text;
	std::wsmatch match;

	auto flushLabelText = [&] {
		if (sublimed_text.empty())
			return;

		append(createLabel(sublimed_text), false);
		sublimed_text.clear();
	};

	auto text = mState.text;

	if (mState.parse_locale_tags)
		text = sky::UnfoldLocaleTags(text);

	auto insertCustomTags = [&] {
		for (const auto& [name, callback] : mTagHandlers)
		{
			std::wregex tag(sky::format(LR"delim(^<{}(?:\s+\w+="(?:\\"|[^"])*")*\s*>)delim", sky::to_wstring(name)));

			if (!std::regex_search(text, match, tag))
				continue;

			std::unordered_map<std::string, std::string> arguments;

			auto attrs_str = match[0].str();
			std::wregex attr_regex(LR"delim((\w+)="((?:\\"|[^"])*)")delim");
			std::wsregex_iterator it(attrs_str.begin(), attrs_str.end(), attr_regex);
			std::wsregex_iterator end;

			for (; it != end; ++it)
			{
				auto attr_match = *it;
				auto name = sky::to_string(attr_match[1].str());
				auto raw_value = attr_match[2].str();
				raw_value = std::regex_replace(raw_value, std::wregex(LR"(\\")"), L"\"");
				auto value = sky::to_string(raw_value);
				arguments.insert({ name, value });
			}

			flushLabelText();
			append(callback(arguments), true);
			text.erase(0, match.length());
			return true;
		}
		return false;
	};

	while (!text.empty())
	{
		if (insertCustomTags())
			continue;

		if (std::regex_search(text, match, icon_tag))
		{
			flushLabelText();
			auto path = sky::to_string(match[1]);
			append(createSprite(path), true);
			text.erase(0, match.length());
		}
		else
		{
			sublimed_text.push_back(text.front());
			text.erase(0, 1);
		}
	}

	if (!sublimed_text.empty())
		append(createLabel(sublimed_text), false);
}

void RichLabel::setTagHandler(const std::string& name, std::function<std::shared_ptr<Node>(const std::unordered_map<std::string, std::string>& args)> callback)
{
	mTagHandlers[name] = callback;
}

void RichLabel::setTagHandler(const std::string& name, std::function<std::shared_ptr<Node>()> callback)
{
	mTagHandlers[name] = [callback](const auto&) {
		return callback();
	};
}
