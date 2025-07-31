#include "scene_helpers.h"
#include <tiny_obj_loader.h>
#include <sstream>
#include <regex>
#include <ranges>
#include "imscene.h"

using namespace Shared;

std::shared_ptr<Scene::Label> SceneHelpers::MakePopupLabel(std::shared_ptr<Scene::Node> holder,
	std::shared_ptr<Scene::Node> target, const std::wstring& text, float text_size, float move_duration)
{
	auto label = std::make_shared<Scene::Label>();
	label->setFontSize(text_size);
	label->setText(text);
	label->setPosition(holder->unproject(target->project({ target->getAbsoluteSize() / 2.0f })));
	label->setPivot(0.5f);
	label->setAlpha(0.0f);
	label->runAction(Actions::Collection::MakeSequence(
		Actions::Collection::Show(label, 0.5f),
		Actions::Collection::ChangePositionByDirection(label, { 0.0f, -1.0f }, 64.0f, move_duration),
		Actions::Collection::Hide(label, 0.5f),
		Actions::Collection::Kill(label)
	));
	holder->attach(label);
	return label;
}

SceneHelpers::WeightedCell::WeightedCell(std::shared_ptr<Scene::Node> _node, float _weight) :
	node(_node), weight(_weight)
{
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeWeightedRow(const std::vector<WeightedCell>& items)
{
	auto row = std::make_shared<Scene::Row>();
	float all = 0.0f;
	for (const auto& item : items)
	{
		all += item.weight;
	}
	for (const auto& item : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		auto stretch_x = item.weight / all;
		cell->setStretch({ stretch_x, 1.0f });
		row->attach(cell);
		cell->attach(item.node);
	}
	return row;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeWeightedColumn(const std::vector<WeightedCell>& items)
{
	auto column = std::make_shared<Scene::Column>();
	float all = 0.0f;
	for (const auto& item : items)
	{
		all += item.weight;
	}
	for (const auto& item : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		auto stretch_y = item.weight / all;
		cell->setStretch({ 1.0f, stretch_y });
		column->attach(cell);
		cell->attach(item.node);
	}
	return column;
}

void SceneHelpers::RecursiveColorSet(std::shared_ptr<Scene::Node> node, const glm::vec4& color)
{
	for (auto child : node->getNodes())
	{
		RecursiveColorSet(child, color);
	}

	auto color_node = std::dynamic_pointer_cast<Scene::Color>(node);

	if (!color_node)
		return;

	color_node->setColor(color);
}

void SceneHelpers::RecursiveAlphaSet(std::shared_ptr<Scene::Node> node, float alpha)
{
	for (auto child : node->getNodes())
	{
		RecursiveAlphaSet(child, alpha);
	}

	auto color_node = std::dynamic_pointer_cast<Scene::Color>(node);

	if (!color_node)
		return;

	color_node->setAlpha(alpha);
}

void SceneHelpers::ParseNodeFromXml(Scene::Node& node, const tinyxml2::XMLElement& root)
{
	auto parseVec2 = [](const char* _str) -> std::optional<glm::vec2> {
		if (!_str)
			return std::nullopt;

		std::string str = _str;

		if (str.empty())
			return std::nullopt;

		std::regex rgx_one_arg(R"(([-+]?[0-9]*\.?[0-9]+))");
		std::regex rgx_two_args(R"(([-+]?[0-9]*\.?[0-9]+)\s*,\s*([-+]?[0-9]*\.?[0-9]+))");
		std::smatch matches;

		if (std::regex_match(str, matches, rgx_two_args))
		{
			auto x = std::stof(matches[1].str());
			auto y = std::stof(matches[2].str());
			return glm::vec2(x, y);
		}

		if (std::regex_match(str, matches, rgx_one_arg))
		{
			auto value = std::stof(matches[1].str());
			return glm::vec2(value, value);
		}

		return std::nullopt;
	};

	node.setAnchor(parseVec2(root.Attribute("anchor")).value_or(node.getAnchor()));
	node.setPivot(parseVec2(root.Attribute("pivot")).value_or(node.getPivot()));
	node.setPosition(parseVec2(root.Attribute("pos")).value_or(node.getPosition()));
	node.setSize(parseVec2(root.Attribute("size")).value_or(node.getSize()));
	node.setStretch(parseVec2(root.Attribute("stretch")).value_or(node.getStretch()));
	node.setMargin(parseVec2(root.Attribute("margin")).value_or(node.getMargin()));
	node.setOrigin(parseVec2(root.Attribute("origin")).value_or(node.getOrigin()));
	node.setX(root.FloatAttribute("x", node.getX()));
	node.setY(root.FloatAttribute("y", node.getY()));
	node.setHeight(root.FloatAttribute("height", node.getHeight()));
	node.setWidth(root.FloatAttribute("width", node.getWidth()));
	node.setHorizontalOrigin(root.FloatAttribute("origin_x", node.getHorizontalOrigin()));
	node.setVerticalOrigin(root.FloatAttribute("origin_y", node.getVerticalOrigin()));

	node.setTouchable(root.BoolAttribute("touchable", node.isTouchable()));

	auto batch_group = root.Attribute("batch_group");

	if (batch_group)
		node.setBatchGroup(batch_group);
}

void SceneHelpers::ParseColorFromXml(Scene::Color& node, const tinyxml2::XMLElement& root)
{
	node.setAlpha(root.FloatAttribute("alpha", node.getAlpha()));
	auto color = root.Attribute("color");
	if (color)
	{
		auto parse_u8_color = [](auto match, float default_alpha) {
			auto r = static_cast<uint8_t>(std::stoi(match[1]));
			auto g = static_cast<uint8_t>(std::stoi(match[2]));
			auto b = static_cast<uint8_t>(std::stoi(match[3]));
			auto a = match.size() > 4 ? static_cast<uint8_t>(std::stoi(match[4])) : static_cast<uint8_t>(default_alpha * 255.0f);
			return sky::ColorToNormalized(r, g, b, a);
		};

		auto parse_float_color = [](auto match, float default_alpha) {
			auto r = std::stof(match[1]);
			auto g = std::stof(match[2]);
			auto b = std::stof(match[3]);
			auto a = match.size() > 4 ? std::stof(match[4]) : default_alpha;
			return glm::vec4{ r, g, b, a };
		};

		auto parse_hex_color = [](auto match, float default_alpha) {
			auto hex_color = match[1];
			auto rgba = std::stoul(hex_color, nullptr, 16);
			auto has_alpha = hex_color.length() == 8;
			auto r = static_cast<uint8_t>((rgba >> (has_alpha ? 24 : 16)) & 0xFF);
			auto g = static_cast<uint8_t>((rgba >> (has_alpha ? 16 : 8)) & 0xFF);
			auto b = static_cast<uint8_t>((rgba >> (has_alpha ? 8 : 0)) & 0xFF);
			auto a = static_cast<uint8_t>(has_alpha ? (rgba & 0xFF) : static_cast<uint8_t>(default_alpha * 255.0f));
			return sky::ColorToNormalized(r, g, b, a);
		};

		std::vector<std::tuple<std::regex, std::function<glm::vec4(std::smatch match, float default_alpha)>>> color_tags = {
			{ std::regex(R"(^rgba\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\))"), parse_u8_color },
			{ std::regex(R"(^rgb\([ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*,[ ]*(\d+)[ ]*\))"), parse_u8_color },
			{ std::regex(R"(^frgba\([ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*\))"), parse_float_color },
			{ std::regex(R"(^frgb\([ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*,[ ]*(\d+|\d+\.\d+)[ ]*\))"), parse_float_color },
			{ std::regex(R"(^hex\([ ]*([0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})[ ]*\))"), parse_hex_color },
		};

		std::smatch match;
		std::string color_str = color;

		for (const auto& [regex, callback] : color_tags)
		{
			if (!std::regex_search(color_str, match, regex))
				continue;

			auto rgba = callback(match, node.getAlpha());
			node.setColor(rgba);
			break;
		}
	}
}

void SceneHelpers::ParseSpriteFromXml(Scene::Sprite& node, const tinyxml2::XMLElement& root)
{
	ParseNodeFromXml(node, root);
	ParseColorFromXml(node, root);
	auto texture = root.Attribute("texture");
	if (texture != nullptr)
	{
		node.setTexture(sky::GetTexture(texture));
	}
}

void SceneHelpers::ParseLabelFromXml(Scene::Label& node, const tinyxml2::XMLElement& root)
{
	ParseNodeFromXml(node, root);
	ParseColorFromXml(node, root);
	node.setFontSize(root.FloatAttribute("font_size", node.getFontSize()));
	auto text = root.Attribute("text");
	if (text != nullptr)
	{
		node.setText(root.BoolAttribute("localized") ? sky::Localize(text) : sky::to_wstring(text));
	}
}

template <class T>
std::shared_ptr<T> CreateNode(const tinyxml2::XMLElement& root)
{
	auto autosize = root.BoolAttribute("autosize");

	bool autowidth = autosize || root.BoolAttribute("autowidth");
	bool autoheight = autosize || root.BoolAttribute("autoheight");

	if (!autowidth && !autoheight)
		return std::make_shared<T>();

	auto result = std::make_shared<Scene::AutoSized<T>>();
	result->setAutoWidthEnabled(autowidth);
	result->setAutoHeightEnabled(autoheight);
	return result;
}

std::unordered_map<std::string, std::function<std::shared_ptr<Scene::Node>(const tinyxml2::XMLElement& root)>> SceneHelpers::XmlCreateFuncs = {
	{ "Node", [](const auto& root) {
		auto node = CreateNode<Scene::Node>(root);
		ParseNodeFromXml(*node, root);
		return node;
	} },
	{ "Sprite", [](const auto& root) {
		auto node = CreateNode<Scene::Sprite>(root);
		ParseSpriteFromXml(*node, root);
		return node;
	} },
	{ "Rectangle", [](const auto& root) {
		auto node = CreateNode<Scene::Rectangle>(root);
		ParseNodeFromXml(*node, root);
		ParseColorFromXml(*node, root);
		node->setRounding(root.FloatAttribute("rounding", node->getRounding()));
		node->setAbsoluteRounding(root.BoolAttribute("absolute_rounding", node->isAbsoluteRounding()));
		return node;
	} },
	{ "Label", [](const auto& root) {
		auto node = CreateNode<Scene::Label>(root);
		ParseLabelFromXml(*node, root);
		return node;
	} },
	{ "RichLabel", [](const auto& root) {
		auto node = CreateNode<Scene::RichLabel>(root);
		ParseNodeFromXml(*node, root);
		node->setFontSize(root.FloatAttribute("font_size", Scene::Label::DefaultFontSize));
		auto text = sky::to_wstring(root.Attribute("text"));
		node->setText(root.BoolAttribute("localized") ? sky::Localize(sky::to_string(text)) : text);
		return node;
	} },
	{ "Column", [](const auto& root) {
		auto node = CreateNode<Scene::Column>(root);
		ParseNodeFromXml(*node, root);
		auto align = root.FloatAttribute("align");
		node->setAlign(align);
		return node;
	} },
};

static std::shared_ptr<Scene::Node> CreateNodesFromXmlElement(const tinyxml2::XMLElement& root,
	std::unordered_map<std::string, std::shared_ptr<Scene::Node>>& collection)
{
	std::shared_ptr<Scene::Node> result;
	std::string name = root.Name();

	if (SceneHelpers::XmlCreateFuncs.contains(name))
		result = SceneHelpers::XmlCreateFuncs.at(name)(root);

	if (!result)
		return nullptr;

	if (auto id = root.Attribute("id"); id != nullptr)
		collection.insert({ id, result });

	for (auto element = root.FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		auto node = CreateNodesFromXmlElement(*element, collection);
		result->attach(node);
	}

	return result;
}

std::tuple<std::shared_ptr<Scene::Node>, std::unordered_map<std::string, std::shared_ptr<Scene::Node>>>
	SceneHelpers::CreateNodesFromXml(const std::string& xml)
{
	tinyxml2::XMLDocument doc;
	auto res = doc.Parse(xml.c_str());

	if (res != tinyxml2::XML_SUCCESS)
		return { nullptr, {} };

	auto root = doc.RootElement();
	std::unordered_map<std::string, std::shared_ptr<Scene::Node>> collection;
	auto node = CreateNodesFromXmlElement(*root, collection);
	return { node, collection };
}

void SceneHelpers::GrayscaleSprite::draw()
{
	static auto effect = sky::effects::Effect<skygfx::utils::effects::Grayscale>();
	effect.uniform.intensity = mGrayIntensity;
	setEffect(&effect);
	Sprite::draw();
}

SceneHelpers::SpriteButton::SpriteButton()
{
	refresh();
}

void SceneHelpers::SpriteButton::refresh()
{
	setGrayIntensity(isActive() ? 0.0f : 1.0f);
}

SceneHelpers::RectangleButton::RectangleButton()
{
	auto label_holder = std::make_shared<Scene::Node>();
	label_holder->setAnchor(0.5f);
	label_holder->setPivot(0.5f);
	label_holder->setStretch(0.75f);
	attach(label_holder);

	mLabel = std::make_shared<Scene::AutoScaled<Scene::Label>>();
	mLabel->setAnchor(0.5f);
	mLabel->setPivot(0.5f);
	label_holder->attach(mLabel);

	refresh();
}

void SceneHelpers::RectangleButton::refresh()
{
	Button<Scene::Rectangle>::refresh();

	if (isActive())
		setColor(mActiveColor);
	else
		setColor(mInactiveColor);
}

void SceneHelpers::RectangleButton::onChooseBegin()
{
	Button<Scene::Rectangle>::onChooseBegin();

	if (!mHighlightEnabled)
		return;

	setColor(mHighlightColor);
}

void SceneHelpers::RectangleButton::onChooseEnd()
{
	Button<Scene::Rectangle>::onChooseEnd();

	if (!mHighlightEnabled)
		return;

	refresh();
}

SceneHelpers::Progressbar::Progressbar()
{
	setAlpha(0.33f);

	mProgressContent = std::make_shared<Scene::Rectangle>();
	mProgressContent->setAnchor({ 0.0f, 0.5f });
	mProgressContent->setPivot({ 0.0f, 0.5f });
	mProgressContent->setVerticalStretch(1.0f);
	mProgressContent->setAlpha(0.66f);
	attach(mProgressContent);
}

void SceneHelpers::Progressbar::setProgress(float value)
{
	mProgressContent->setHorizontalStretch(glm::clamp(value, 0.0f, 1.0f));
}

float SceneHelpers::Progressbar::getProgress() const
{
	return mProgressContent->getHorizontalStretch();
}

// safe area

SceneHelpers::SafeArea::SafeArea()
{
	setStretch(1.0f);
}

void SceneHelpers::SafeArea::update(sky::Duration dTime)
{
	Scene::Node::update(dTime);

	auto scale = PLATFORM->getScale();
	auto left = PLATFORM->getSafeAreaLeftMargin() / scale;
	auto top = PLATFORM->getSafeAreaTopMargin() / scale;
	auto bottom = PLATFORM->getSafeAreaBottomMargin() / scale;
	auto right = PLATFORM->getSafeAreaRightMargin() / scale;

	setVerticalPosition(top);
	setHorizontalPosition(left);
	setVerticalMargin(top + bottom);
	setHorizontalMargin(left + right);
}

SceneHelpers::VerticalScrollbar::VerticalScrollbar()
{
	setStretch({ 0.0f, 1.0f });
	setMargin({ 0.0f, 8.0f });
	setAnchor({ 1.0f, 0.5f });
	setPivot({ 1.0f, 0.5f });
	setPosition({ -8.0f, 0.0f });
	setSize({ 2.0f, 0.0f });
	setAlpha(BarAlpha);
	setRounding(1.0f);

	mIndicator = std::make_shared<Scene::Rectangle>();
	mIndicator->setStretch({ 1.0f, 0.0f });
	mIndicator->setAnchor({ 0.5f, 0.0f });
	mIndicator->setPivot({ 0.5f, 0.0f });
	mIndicator->setRounding(1.0f);
	mIndicator->setAlpha(IndicatorAlpha);
	attach(mIndicator);
}

void SceneHelpers::VerticalScrollbar::update(sky::Duration dTime)
{
	Scene::Rectangle::update(dTime);

	setVisible(false);

	if (mScrollbox.expired())
		return;

	auto scrollbox = mScrollbox.lock();
	auto v_scroll_pos = scrollbox->getVerticalScrollPosition();

	mIndicator->setVerticalAnchor(v_scroll_pos);
	mIndicator->setVerticalPivot(v_scroll_pos);
	mIndicator->setVerticalStretch(scrollbox->getBounding()->getAbsoluteHeight() /
		scrollbox->getContent()->getAbsoluteHeight());

	if (mIndicator->getVerticalStretch() >= 1.0f)
		return;

	setVisible(true);

	auto now = sky::Now();

	if (mPrevScrollPosition != v_scroll_pos)
		mScrollMoveTime = now;

	mPrevScrollPosition = v_scroll_pos;

	if (!mHidingEnabled)
		return;

	if (mAlphaAnimating)
		return;

	const float Timeout = 0.5f;
	const float AnimDuration = 0.25f;

	bool visible = now - mScrollMoveTime < sky::FromSeconds(Timeout) || scrollbox->isTouching();

	if (visible)
	{
		if (!mHidden)
			return;

		mAlphaAnimating = true;
		runAction(Actions::Collection::MakeSequence(
			Actions::Collection::MakeParallel(
				Actions::Collection::ChangeAlpha(shared_from_this(), BarAlpha, AnimDuration, Easing::CubicInOut),
				Actions::Collection::ChangeAlpha(mIndicator, IndicatorAlpha, AnimDuration, Easing::CubicInOut)
			),
			Actions::Collection::Execute([this] {
				mHidden = false;
				mAlphaAnimating = false;
			})
		));
	}
	else
	{
		if (mHidden)
			return;

		mAlphaAnimating = true;
		runAction(Actions::Collection::MakeSequence(
			Actions::Collection::MakeParallel(
				Actions::Collection::ChangeAlpha(shared_from_this(), 0.0f, AnimDuration, Easing::CubicInOut),
				Actions::Collection::ChangeAlpha(mIndicator, 0.0f, AnimDuration, Easing::CubicInOut)
			),
			Actions::Collection::Execute([this] {
				mHidden = true;
				mAlphaAnimating = false;
			})
		));
	}
}

// standard screen

SceneHelpers::StandardScreen::StandardScreen(const std::set<Effect>& effects) :
	mEffects(effects)
{
	setEnabled(false);
	setInteractions(false);
	setStretch(1.0f);
	setAnchor(0.5f);
	setPivot(0.5f);

	mContent = std::make_shared<Scene::Node>();
	mContent->setStretch(1.0f);
	mContent->setAnchor(0.5f);
	mContent->setPivot(0.5f);
	attach(mContent);

	if (mEffects.contains(Effect::Blur))
	{
		mBlur = std::make_shared<Scene::BlurredGlass>();
		mBlur->setStretch(1.0f);
		attach(mBlur);
	}
}

void SceneHelpers::StandardScreen::onEnterBegin()
{
	setEnabled(true);
	setRenderLayerEnabled(true);
	if (mEffects.contains(Effect::Blur))
	{
		mBlur->setBlurIntensity(1.0f);
	}

	if (mEffects.contains(Effect::Scale))
	{
		mContent->setScale(0.95f);
	}

	if (mEffects.contains(Effect::Alpha))
	{
		getRenderLayerColor()->setAlpha(0.0f);
	}
}

void SceneHelpers::StandardScreen::onEnterEnd()
{
	setInteractions(true);
	setRenderLayerEnabled(false);
}

void SceneHelpers::StandardScreen::onLeaveBegin()
{
	setInteractions(false);
	setRenderLayerEnabled(true);
}

void SceneHelpers::StandardScreen::onLeaveEnd()
{
	setEnabled(false);
	setRenderLayerEnabled(false);
}

void SceneHelpers::StandardScreen::onWindowAppearingBegin()
{
	setInteractions(false);
	if (mEffects.contains(Effect::WindowAppearingScale))
	{
		runAction(Actions::Collection::ChangeScale(mContent, StartScale, 1.0f, Easing::CubicOut));
	}
}

void SceneHelpers::StandardScreen::onWindowDisappearingBegin()
{
	if (mEffects.contains(Effect::WindowAppearingScale))
	{
		runAction(Actions::Collection::ChangeScale(mContent, { 1.0f, 1.0f }, 1.0f, Easing::CubicOut));
	}
}

void SceneHelpers::StandardScreen::onWindowDisappearingEnd()
{
	setInteractions(true);
}

std::unique_ptr<Actions::Action> SceneHelpers::StandardScreen::createEnterAction()
{
	const float Duration = 0.25f;

	auto parallel = Actions::Collection::MakeParallel();

	if (mEffects.contains(Effect::Alpha))
	{
		parallel->add(Actions::Collection::Show(getRenderLayerColor(), Duration));
	}

	if (mEffects.contains(Effect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mBlur, 0.0f, Duration, Easing::Linear));
	}

	if (mEffects.contains(Effect::Scale))
	{
		parallel->add(Actions::Collection::ChangeScale(mContent, { 1.0f, 1.0f }, Duration, Easing::CubicOut));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitGlobalFrame(),
		std::move(parallel)
	);
};

std::unique_ptr<Actions::Action> SceneHelpers::StandardScreen::createLeaveAction()
{
	const float Duration = 0.25f;

	auto parallel = Actions::Collection::MakeParallel();

	if (mEffects.contains(Effect::Alpha))
	{
		parallel->add(Actions::Collection::Hide(getRenderLayerColor(), Duration));
	}

	if (mEffects.contains(Effect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mBlur, 1.0f, Duration, Easing::Linear));
	}

	if (mEffects.contains(Effect::Scale))
	{
		parallel->add(Actions::Collection::ChangeScale(mContent, StartScale, Duration, Easing::CubicIn));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitGlobalFrame(),
		std::move(parallel)
	);
};

// standard window

SceneHelpers::StandardWindow::StandardWindow(const std::set<BackgroundEffect> background_effect,
	const std::set<ContentEffect> content_effect) : mBackgroundEffect(background_effect), mContentEffect(content_effect)
{
	setStretch(1.0f);
	setClickCallback([this] {
		if (!mCloseOnMissclick)
			return;

		if (getState() != State::Opened)
			return;

		SCENE_MANAGER->popWindow();
	});

	mContentHolder = std::make_shared<Scene::RenderLayer<Scene::Node>>();
	mContentHolder->setStretch(1.0f);
	mContentHolder->setAnchor(0.5f);
	mContentHolder->setPivot(0.5f);
	mContentHolder->setInteractions(false);
	attach(mContentHolder);

	mContent = std::make_shared<Scene::Node>();
	mContent->setStretch(1.0f);
	mContent->setAnchor(0.5f);
	mContent->setPivot(0.5f);
	mContentHolder->attach(mContent);

	getBackshadeColor()->setColor({ sky::GetColor(sky::Color::Black), 0.0f });

	if (mBackgroundEffect.contains(BackgroundEffect::Blur))
	{
		mBlur = std::make_shared<Scene::BlurredGlass>();
		mBlur->setStretch(1.0f);
		mBlur->setBlurIntensity(0.0f);
		attach(mBlur, Scene::Node::AttachDirection::Front);
	}

	if (mBackgroundEffect.contains(BackgroundEffect::Gray))
	{
		mGray = std::make_shared<Scene::GrayscaledGlass>();
		mGray->setStretch(1.0f);
		mGray->setGrayscaleIntensity(0.0f);
		attach(mGray, Scene::Node::AttachDirection::Front);
	}

	if (mContentEffect.contains(ContentEffect::Blur))
	{
		mContentBlur = std::make_shared<Scene::BlurredGlass>();
		mContentBlur->setStretch(1.0f);
		mContentBlur->setBlurIntensity(StartContentBlur);
		mContentHolder->attach(mContentBlur);
	}

	if (mContentEffect.contains(ContentEffect::Anchor))
	{
		mContent->setAnchor(StartContentAnchor);
	}

	if (mContentEffect.contains(ContentEffect::Alpha))
	{
		mContentHolder->getRenderLayerColor()->setAlpha(0.0f);
	}

	if (mContentEffect.contains(ContentEffect::Scale))
	{
		mContentHolder->setScale(StartContentScale);
	}
}

void SceneHelpers::StandardWindow::onOpenEnd()
{
	mContentHolder->setInteractions(true);
	mContentHolder->setRenderLayerEnabled(false);
}

void SceneHelpers::StandardWindow::onCloseBegin()
{
	mContentHolder->setInteractions(false);
	mContentHolder->setRenderLayerEnabled(true);
}

std::unique_ptr<Actions::Action> SceneHelpers::StandardWindow::createOpenAction()
{
	const float Duration = 0.5f;

	auto parallel = Actions::Collection::MakeParallel();

	if (mBackgroundEffect.contains(BackgroundEffect::Fade))
	{
		parallel->add(Actions::Collection::ChangeAlpha(getBackshadeColor(), 0.5f, Duration, Easing::CubicOut));
	}

	if (mBackgroundEffect.contains(BackgroundEffect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mBlur, 1.0f, Duration, Easing::CubicOut));
		parallel->add(Actions::Collection::ChangeColorRgb(mBlur, glm::vec3(1.0f + (0.125f / 2.0f)), Duration, Easing::CubicOut));
	}

	if (mBackgroundEffect.contains(BackgroundEffect::Gray))
	{
		parallel->add(Actions::Collection::ChangeGrayscaleIntensity(mGray, 1.0f, Duration, Easing::CubicOut));
	}

	if (mContentEffect.contains(ContentEffect::Anchor))
	{
		parallel->add(Actions::Collection::ChangeAnchor(mContent, { 0.5f, 0.5f }, Duration, Easing::CubicOut));
	}

	if (mContentEffect.contains(ContentEffect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mContentBlur, 0.0f, Duration, Easing::Linear));
	}

	if (mContentEffect.contains(ContentEffect::Alpha))
	{
		parallel->add(Actions::Collection::Show(mContentHolder->getRenderLayerColor(), Duration, Easing::CubicOut));
	}

	if (mContentEffect.contains(ContentEffect::Scale))
	{
		parallel->add(Actions::Collection::ChangeScale(mContentHolder, { 1.0f, 1.0f }, Duration, Easing::CubicOut));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitGlobalFrame(),
		std::move(parallel)
	);
};

std::unique_ptr<Actions::Action> SceneHelpers::StandardWindow::createCloseAction()
{
	const float Duration = 0.5f;

	auto parallel = Actions::Collection::MakeParallel();

	if (mBackgroundEffect.contains(BackgroundEffect::Fade))
	{
		parallel->add(Actions::Collection::ChangeAlpha(getBackshadeColor(), 0.0f, Duration, Easing::CubicIn));
	}

	if (mBackgroundEffect.contains(BackgroundEffect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mBlur, 0.0f, Duration, Easing::CubicIn));
		parallel->add(Actions::Collection::ChangeColorRgb(mBlur, glm::vec3(1.0f), Duration, Easing::CubicIn));
	}

	if (mBackgroundEffect.contains(BackgroundEffect::Gray))
	{
		parallel->add(Actions::Collection::ChangeGrayscaleIntensity(mGray, 0.0f, Duration, Easing::CubicIn));
	}

	if (mContentEffect.contains(ContentEffect::Anchor))
	{
		parallel->add(Actions::Collection::ChangeAnchor(mContent, StartContentAnchor, Duration, Easing::CubicIn));
	}

	if (mContentEffect.contains(ContentEffect::Blur))
	{
		parallel->add(Actions::Collection::ChangeBlurIntensity(mContentBlur, StartContentBlur, Duration, Easing::Linear));
	}

	if (mContentEffect.contains(ContentEffect::Alpha))
	{
		parallel->add(Actions::Collection::Hide(mContentHolder->getRenderLayerColor(), Duration, Easing::CubicIn));
	}

	if (mContentEffect.contains(ContentEffect::Scale))
	{
		parallel->add(Actions::Collection::ChangeScale(mContentHolder, StartContentScale, Duration, Easing::CubicIn));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitGlobalFrame(),
		std::move(parallel)
	);
};

// blurred glass demo

SceneHelpers::BlurredGlassDemo::BlurredGlassDemo()
{
	auto scroll_bg = std::make_shared<Scene::Rectangle>();
	scroll_bg->setStretch({ 1.0f, 0.0f });
	scroll_bg->setHeight(8.0f);
	scroll_bg->setAnchor({ 0.5f, 1.0f });
	scroll_bg->setPivot({ 0.5f, 0.0f });
	scroll_bg->setY(4.0f);
	scroll_bg->setAlpha(0.25f);
	attach(scroll_bg);

	auto slider = std::make_shared<Scene::Rectangle>();
	slider->setPivot(0.5f);
	slider->setAnchor(0.5f);
	slider->setStretch({ 0.0f, 1.0f });
	slider->setWidth(8.0f);

	auto scrollbox = std::make_shared<Scene::Scrollbox>();
	scrollbox->setStretch(1.0f);
	scrollbox->setOverscrollEnabled(false);
	scrollbox->getBounding()->setStretch(1.0f);
	scrollbox->getBounding()->setAnchor(0.5f);
	scrollbox->getBounding()->setPivot(0.5f);
	scrollbox->getContent()->setStretch({ 2.0f, 1.0f });
	scrollbox->getContent()->attach(slider);
	scroll_bg->attach(scrollbox);

	auto label = std::make_shared<Scene::Label>();
	label->setPosition({ 2.0f, 2.0f });
	label->setFontSize(12.0f);
	attach(label);

	runAction(Actions::Collection::ExecuteInfinite([this, scrollbox, slider, label] {
		if (!scrollbox->isTouching() && !scrollbox->isInerting() && !scrollbox->isPullbacking())
			scrollbox->setHorizontalScrollPosition(1.0f - getBlurIntensity());

		auto h_pos = scrollbox->getHorizontalScrollPosition();
		setBlurIntensity(1.0f - h_pos);
		slider->setHorizontalPivot(1.0f - h_pos);
		label->setText(sky::format(L"{:.{}f}", getBlurIntensity(), 2));
	}));
}

// shockwave

std::shared_ptr<SceneHelpers::Shockwave> SceneHelpers::Shockwave::MakeAnimated(float duration)
{
	auto shockwave = std::make_shared<Shockwave>();
	shockwave->setShockwaveSize(0.0f);
	shockwave->setShockwaveThickness(0.5f);
	shockwave->setShockwaveForce(1.0f);
	shockwave->runAction(Actions::Collection::MakeSequence(
		Actions::Collection::MakeParallel(
			Actions::Collection::Interpolate(shockwave->getShockwaveSize(), 1.0f, duration, Easing::SinusoidalOut, [shockwave](float value) {
				shockwave->setShockwaveSize(value);
			}),
			Actions::Collection::Interpolate(shockwave->getShockwaveForce(), 0.0f, duration, Easing::SinusoidalOut, [shockwave](float value) {
				shockwave->setShockwaveForce(value);
			})
		),
		Actions::Collection::Kill(shockwave)
	));
	return shockwave;
}

SceneHelpers::Shockwave::Shockwave()
{
	setBlendMode(skygfx::BlendStates::Opaque);
}

void SceneHelpers::Shockwave::draw()
{
	static auto effect = sky::effects::Effect<sky::effects::Shockwave>();
	effect.uniform.size = mShockwaveSize;
	effect.uniform.thickness = mShockwaveThickness;
	effect.uniform.force = mShockwaveForce;

	setEffect(&effect);

	Glass::draw();
}

// checkbox

SceneHelpers::Checkbox::Checkbox()
{
	mOuterRectangle = std::make_shared<Scene::Rectangle>();
	mOuterRectangle->setAlpha(0.33f);
	mOuterRectangle->setStretch({ 0.0f, 1.0f });
	mOuterRectangle->setMargin(8.0f);
	mOuterRectangle->setX(4.0f);
	mOuterRectangle->setAnchor({ 0.0f, 0.5f });
	mOuterRectangle->setPivot({ 0.0f, 0.5f });
	attach(mOuterRectangle);

	mOuterRectangle->runAction(Actions::Collection::ExecuteInfinite([this] {
		mOuterRectangle->setWidth(mOuterRectangle->getAbsoluteHeight() + mOuterRectangle->getVerticalMargin());
	}));

	mInnerRectangle = std::make_shared<Scene::Rectangle>();
	mInnerRectangle->setAnchor(0.5f);
	mInnerRectangle->setPivot(0.5f);
	mInnerRectangle->setStretch(0.66f);
	mInnerRectangle->setAlpha(0.66f);
	mOuterRectangle->attach(mInnerRectangle);

	mInnerRectangle->runAction(Actions::Collection::ExecuteInfinite([this] {
		mInnerRectangle->setVisible(isChecked());
	}));

	mLabel = std::make_shared<Scene::Label>();
	mLabel->setFontSize(12.0f);
	mLabel->setText(L"label");
	mLabel->setAnchor({ 1.0f, 0.5f });
	mLabel->setPivot({ 0.0f, 0.5f });
	mLabel->setX(8.0f);
	mOuterRectangle->attach(mLabel);

	setClickCallback([this] {
		mChecked = !mChecked;
		if (mChangeCallback)
			mChangeCallback(mChecked);
	});
	setChooseBeginCallback([this] { mOuterRectangle->setAlpha(0.66f); });
	setChooseEndCallback([this] { mOuterRectangle->setAlpha(0.33f); });
}

// editbox

SceneHelpers::Editbox::Editbox()
{
	setColor({ 0.25f, 0.25f, 0.25f, 1.0f });
	getLabel()->setFontSize(16.0f);

	setClickCallback([]{
		auto window = std::make_shared<Window>();
		SCENE_MANAGER->pushWindow(window);
	});
}

SceneHelpers::Editbox::Window::Window()
{
	auto label = std::make_shared<Scene::Label>();
	label->setAnchor(0.5f);
	label->setPivot(0.5f);
	label->setText(L"test test test");
	getContent()->attach(label);
}

SceneHelpers::CursorIndicator::CursorIndicator(std::shared_ptr<Scene::Label> label)
{
	setAlpha(0.0f);
	setWidth(2.0f);
	setRounding(1.0f);
	runAction(Actions::Collection::ExecuteInfinite([this, label] {
		auto font = label->getFont();

		auto font_scale = font->getScaleFactorForSize(label->getFontSize());
		auto height = font->getAscent() * font_scale;
		height -= font->getDescent() * font_scale;

		setHeight(height);

		if (label->getText().empty())
		{
			setPosition({ 0.0f, 0.0f });
			setAnchor(0.5f);
			setPivot(0.5f);
			return;
		}

		setAnchor(0.0f);
		setPivot({ 0.5f, 0.0f });

		auto index = mCursorPos.value_or(label->getText().length() - 1);

		if (index > label->getText().length() - 1)
			index = label->getText().length() - 1;

		auto [pos, size] = label->getSymbolBounds(index);
		auto line_y = label->getSymbolLineY(index);

		setX(pos.x + size.x);
		setY(line_y);
	}));

	runAction(Actions::Collection::MakeSequence(
		Actions::Collection::WaitGlobalFrame(),
		Actions::Collection::Execute([this] {
			runAction(Actions::Collection::RepeatInfinite([this] {
				return Actions::Collection::MakeSequence(
					Actions::Collection::ChangeAlpha(shared_from_this(), 1.0f, 0.125f),
					Actions::Collection::Wait(0.25f),
					Actions::Collection::ChangeAlpha(shared_from_this(), 0.0f, 0.125f),
					Actions::Collection::Wait(0.25f)
				);
			}));
		})
	));
}

bool SceneHelpers::ImScene::IsMouseHovered(Scene::Node& node)
{
	if (!node.isTransformReady())
		return false;

	auto cursor_pos = PLATFORM->getCursorPos();

	if (!cursor_pos.has_value())
		return false;

	if (!node.getScene()->interactTest(cursor_pos.value()))
		return false;

	static std::unordered_set<Scene::Node*> touched_nodes_set;
	static std::optional<uint64_t> prev_frame;

	auto current_frame = SCHEDULER->getFrameCount();

	if (prev_frame != current_frame)
	{
		// do it one time per frame, because it is very heavyweight function
		auto touched_nodes = node.getScene()->getTouchedNodes(cursor_pos.value());
		touched_nodes_set.clear();
		std::ranges::transform(touched_nodes, std::inserter(touched_nodes_set, touched_nodes_set.begin()), [](const auto& node) {
			return node.lock().get();
		});
		prev_frame = current_frame;
	}

	return touched_nodes_set.contains(&node);
}

void SceneHelpers::ImScene::Tooltip(Scene::Node& holder,
	std::function<std::shared_ptr<Scene::Node>()> createContentCallback, std::optional<std::string> key)
{
	if (!holder.isTransformReady())
		return;

	auto cursor_pos = PLATFORM->getCursorPos();

	if (!cursor_pos.has_value())
		return;

	auto pos = holder.unproject(cursor_pos.value());

	auto rect = IMSCENE->spawn<Scene::Eased<Scene::AutoSized<Scene::ClippableScissor<Scene::Rectangle>>>>(holder, key);
	if (IMSCENE->isFirstCall())
	{
		rect->setAbsoluteRounding(true);
		rect->setRounding(16.0f);
		rect->setColor(sky::GetColor(sky::Color::Black));
		rect->setAlpha(0.5f);
		rect->setPivot({ 0.0f, 1.0f });
		rect->setOrigin({ -16.0f, 16.0f });
		rect->setMargin(-24.0f);
		rect->setPosition(pos);
		rect->attach(createContentCallback());
	}

	rect->setPosition(pos);
}

void SceneHelpers::ImScene::Tooltip(Scene::Node& holder, const std::wstring& text)
{
	Tooltip(holder, [&] {
		auto label = std::make_shared<Scene::Label>();
		label->setAnchor(0.5f);
		label->setPivot(0.5f);
		label->setFontSize(26.0f);
		label->setText(text);
		return label;
	}, sky::to_string(text));
}

void SceneHelpers::ImScene::HighlightUnderCursor(Scene::Node& holder, Scene::Node& node, HighlightSettings settings, std::optional<std::string> key)
{
	if (!IsMouseHovered(node))
		return;

	auto bounds = node.getGlobalBounds();

	auto rect = IMSCENE->spawn<Scene::Eased<Shared::SceneHelpers::Outlined<Scene::Rectangle>>>(holder, key);
	if (IMSCENE->isFirstCall())
	{
		rect->setColor(settings.rect_color);
		rect->getOutlineColor()->setColor(settings.outline_color);
	}
	rect->setPosition(holder.unproject(bounds.pos));
	rect->setSize(holder.unproject(bounds.size));
}
