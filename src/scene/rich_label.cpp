#include "rich_label.h"
#include <scene/adaptive.h>
#include <scene/sprite.h>
#include <regex>
#include <common/helpers.h>

using namespace Scene;

RichLabel::RichLabel()
{
	mContent = std::make_shared<AutoSized<Row>>();
	mContent->setAnchor(0.5f);
	mContent->setPivot(0.5f);
	attach(mContent);
}

void RichLabel::update(Clock::Duration dTime)
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
		sprite->setTexture(DefaultIconTextureCallback(path));
		sprite->setAdaptStretch(1.0f);
		sprite->setBakingAdaption(true);
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

	auto insertCustomTags = [&] {
		for (const auto& [name, callback] : mTags)
		{
			std::wregex tag(std::format(L"^<{}>", sky::to_wstring(name)));
			if (std::regex_search(text, match, tag))
			{
				flushLabelText();
				append(callback(), true);
				text.erase(0, match.length());
				return true;
			}
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

void RichLabel::setTag(const std::string& name, std::function<std::shared_ptr<Node>()> callback)
{
	mTags[name] = callback;
}
