#include "scene_helpers.h"
#include <shared/cache_system.h>

using namespace Shared;

std::shared_ptr<Scene::Label> SceneHelpers::MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
	std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration)
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

std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> SceneHelpers::MakeFastCheckbox(
	const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback)
{
	auto holder = std::make_shared<Scene::Clickable<Scene::Node>>();

	auto outer_rect = std::make_shared<Scene::Rectangle>();
	outer_rect->setAlpha(0.33f);
	outer_rect->setStretch({ 0.0f, 1.0f });
	outer_rect->setMargin(8.0f);
	outer_rect->setAnchor({ 0.0f, 0.5f });
	outer_rect->setPivot({ 0.0f, 0.5f });
	holder->attach(outer_rect);

	outer_rect->runAction(Actions::Collection::ExecuteInfinite([outer_rect] {
		outer_rect->setWidth(outer_rect->getAbsoluteHeight() + outer_rect->getVerticalMargin());
	}));

	auto inner_rect = std::make_shared<Scene::Rectangle>();
	inner_rect->setAnchor(0.5f);
	inner_rect->setPivot(0.5f);
	inner_rect->setStretch(0.66f);
	inner_rect->setAlpha(0.66f);
	inner_rect->setEnabled(checked);
	outer_rect->attach(inner_rect);

	auto label = std::make_shared<Scene::Label>();
	label->setFontSize(title_size);
	label->setText(title);
	label->setAnchor({ 1.0f, 0.5f });
	label->setPivot({ 0.0f, 0.5f });
	label->setX(8.0f);
	outer_rect->attach(label);

	holder->setClickCallback([inner_rect, changeCallback] { 
		inner_rect->setEnabled(!inner_rect->isEnabled());
		changeCallback(inner_rect->isEnabled());
	});
	holder->setChooseBeginCallback([outer_rect] { outer_rect->setAlpha(0.66f); });
	holder->setChooseEndCallback([outer_rect] { outer_rect->setAlpha(0.33f); });

	auto setter = [inner_rect](bool value) {
		inner_rect->setEnabled(value);
	};

	return { holder, setter };
}

std::vector<std::shared_ptr<Scene::Node>> SceneHelpers::MakeFastRadioButtons(std::vector<utf8_string> titles,
	float title_size, int choosed, std::function<void(int)> changeCallback)
{
	std::vector<std::shared_ptr<Scene::Node>> result;
	auto setters = std::make_shared<std::vector<std::function<void(bool)>>>();

	int index = 0;

	for (const auto& title : titles)
	{
		auto [checkbox, setter] = MakeFastCheckbox(title, title_size, choosed == index, [index, setters, changeCallback](bool value) {
			for (auto setter : *setters)
				setter(false);
			
			setters->at(index)(true);
			changeCallback(index);
		});

		setters->push_back(setter);
		result.push_back(checkbox);

		index += 1;
	}

	return result;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeHorizontalGrid(float height, 
	const std::vector<std::pair<float/*width*/, std::shared_ptr<Scene::Node>>>& items)
{
	auto holder = std::make_shared<Scene::Node>();
	holder->setHeight(height);

	float x = 0.0f;

	for (auto [width, node] : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setHeight(height);
		cell->setWidth(width);
		cell->setX(x);
		holder->attach(cell);
		
		cell->attach(node);

		x += width;
	}

	holder->setWidth(x);

	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeHorizontalGrid(const glm::vec2& cell_size, 
	const std::vector<std::shared_ptr<Scene::Node>>& items)
{
	auto items_width = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_width.push_back({ cell_size.x, item });
	}

	return MakeHorizontalGrid(cell_size.y, items_width);
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(float width,
	const std::vector<std::pair<float/*height*/, std::shared_ptr<Scene::Node>>>& items)
{
	auto holder = std::make_shared<Scene::Node>();
	holder->setWidth(width);

	float y = 0.0f;

	for (auto [height, node] : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setHeight(height);
		cell->setWidth(width);
		cell->setY(y);
		holder->attach(cell);

		cell->attach(node);

		y += height;
	}

	holder->setHeight(y);

	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(const glm::vec2& cell_size,
	const std::vector<std::shared_ptr<Scene::Node>>& items)
{
	auto items_height = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_height.push_back({ cell_size.y, item });
	}

	return MakeVerticalGrid(cell_size.x, items_height);
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

SceneHelpers::GrayscaleSprite::GrayscaleSprite()
{
	if (!Shader)
		Shader = std::make_shared<Renderer::Shaders::Grayscale>(Renderer::Vertex::PositionColorTexture::Layout);

	setShader(Shader);
}

void SceneHelpers::GrayscaleSprite::draw()
{
	Shader->setIntensity(mGrayIntensity);
	Sprite::draw();
}

SceneHelpers::GrayscaleSpriteButton::GrayscaleSpriteButton()
{
	refresh();
}

void SceneHelpers::GrayscaleSpriteButton::refresh()
{
	setGrayIntensity(isActive() ? 0.0f : 1.0f);
}

SceneHelpers::RectangleButton::RectangleButton()
{
	mLabel = std::make_shared<Scene::Label>();
	mLabel->setAnchor(0.5f);
	mLabel->setPivot(0.5f);
	attach(mLabel);

	refresh();
}

void SceneHelpers::RectangleButton::refresh()
{
	setAlpha(isActive() ? 0.33f : 0.125f);
	setClickEnabled(isActive());
}

void SceneHelpers::RectangleButton::onChooseBegin()
{
	Button<Scene::Rectangle>::onChooseBegin();

	if (!mHighlightEnabled)
		return;

	setAlpha(0.66f);
}

void SceneHelpers::RectangleButton::onChooseEnd()
{
	Button<Scene::Rectangle>::onChooseEnd();

	if (!mHighlightEnabled)
		return;

	setAlpha(0.33f);
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
	mProgressContent->setHorizontalStretch(value);
}

float SceneHelpers::Progressbar::getProgress() const
{
	return mProgressContent->getHorizontalStretch();
}

SceneHelpers::Hud::Hud()
{
	setStretch(1.0f);
}

void SceneHelpers::Hud::update()
{
	Scene::Node::update();

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
	mIndicator->setSize({ 0.0f, 32.0f });
	mIndicator->setStretch({ 1.0f, 0.0f });
	mIndicator->setAnchor({ 0.5f, 0.0f });
	mIndicator->setPivot({ 0.5f, 0.0f });
	mIndicator->setRounding(1.0f);
	mIndicator->setAlpha(IndicatorAlpha);
	attach(mIndicator);
}

void SceneHelpers::VerticalScrollbar::update()
{
	Scene::Rectangle::update();

	if (mScrollbox.expired())
	{
		setVisible(false);
		return;
	}

	setVisible(true);

	auto scrollbox = mScrollbox.lock();
	auto v_scroll_pos = scrollbox->getVerticalScrollPosition();

	mIndicator->setVerticalAnchor(v_scroll_pos);
	mIndicator->setVerticalPivot(v_scroll_pos);

	auto now = Clock::Now();

	if (mPrevScrollPosition != v_scroll_pos)
		mScrollMoveTime = now;

	mPrevScrollPosition = v_scroll_pos;

	if (mAlphaAnimating)
		return;

	const float Timeout = 0.5f;
	const float AnimDuration = 0.25f;

	bool visible = now - mScrollMoveTime < Clock::FromSeconds(Timeout) || scrollbox->isTouching();

	if(visible)
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