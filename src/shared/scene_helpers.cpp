#include "scene_helpers.h"
#include <shared/cache_system.h>
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

void SceneHelpers::SafeArea::update(Clock::Duration dTime)
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

void SceneHelpers::VerticalScrollbar::update(Clock::Duration dTime)
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

	auto now = Clock::Now();

	if (mPrevScrollPosition != v_scroll_pos)
		mScrollMoveTime = now;

	mPrevScrollPosition = v_scroll_pos;

	if (!mHidingEnabled)
		return;

	if (mAlphaAnimating)
		return;

	const float Timeout = 0.5f;
	const float AnimDuration = 0.25f;

	bool visible = now - mScrollMoveTime < Clock::FromSeconds(Timeout) || scrollbox->isTouching();

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

	getBackshadeColor()->setColor({ Graphics::Color::Black, 0.0f });

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
		parallel->add(Actions::Collection::ChangeColor(mBlur, glm::vec3(1.0f + (0.125f / 2.0f)), Duration, Easing::CubicOut));
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
		parallel->add(Actions::Collection::ChangeColor(mBlur, glm::vec3(1.0f), Duration, Easing::CubicIn));
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
		label->setText(fmt::format(L"{:.{}f}", getBlurIntensity(), 2));
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

// 3d

/*std::vector<std::shared_ptr<Scene3D::Model>> SceneHelpers::MakeModelsFromObj(const std::string& path_to_folder, const std::string& name_without_extension)
{
	std::vector<std::shared_ptr<Scene3D::Model>> result;

	auto folder_path_with_slash = path_to_folder;

	if (!folder_path_with_slash.ends_with("/"))
		folder_path_with_slash += "/";

	auto obj_asset = Platform::Asset(folder_path_with_slash + name_without_extension + ".obj");
	auto mtl_asset = Platform::Asset(folder_path_with_slash + name_without_extension + ".mtl");

	std::string warn;
	std::string err;

	std::stringstream obj_stream;
	obj_stream.write((const char*)obj_asset.getMemory(), obj_asset.getSize());

	std::stringstream mtl_stream;
	mtl_stream.write((const char*)mtl_asset.getMemory(), mtl_asset.getSize());
	tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &obj_stream, &mtl_reader);

	for (const auto& shape : shapes)
	{
		auto material_id = shape.mesh.material_ids[0];
		auto material = materials.at(material_id);

		auto model = std::make_shared<Scene3D::Model>();

		bool has_texture = !material.diffuse_texname.empty();

		if (has_texture)
			model->setTexture(TEXTURE(folder_path_with_slash + material.diffuse_texname));

		auto positions = Scene3D::Model::PositionAttribs();
		auto normals = Scene3D::Model::NormalAttribs();
		auto texcoords = Scene3D::Model::TexCoordAttribs();
		auto colors = Scene3D::Model::ColorAttribs();

		for (auto index : shape.mesh.indices)
		{
			positions.push_back(*(glm::vec3*)&attrib.vertices.at(index.vertex_index * 3));
			normals.push_back(*(glm::vec3*)&attrib.normals.at(index.normal_index * 3));
			auto texcoord = *(glm::vec2*)&attrib.texcoords.at(index.texcoord_index * 2);
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
			colors.push_back({ Graphics::Color::White, 1.0f });
		}

		model->setPositionAttribs(positions);
		model->setNormalAttribs(normals);

		if (has_texture)
			model->setTexCoordAttribs(texcoords);

		model->setColorAttribs(colors);

		result.push_back(model);
	}

	return result;
}
*/

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

SceneHelpers::RichLabel::RichLabel()
{
	mContent = std::make_shared<Scene::AutoSized<Scene::Row>>();
	mContent->setAnchor(0.5f);
	mContent->setPivot(0.5f);
	attach(mContent);
}

void SceneHelpers::RichLabel::update(Clock::Duration dTime)
{
	AutoSized<Scene::Node>::update(dTime);
	refresh();

	Shared::SceneHelpers::RecursiveColorSet(mContent, getColor());
}

void SceneHelpers::RichLabel::refresh()
{
	if (mState == mPrevState)
		return;

	mPrevState = mState;
	mContent->clear();

	auto append = [&](std::shared_ptr<Scene::Node> item, bool stretch) {
		auto holder = std::make_shared<Scene::AutoSized<Scene::Node>>();
		holder->setAnchor({ 0.0f, 0.5f });
		holder->setPivot({ 0.0f, 0.5f });
		holder->setAutoHeightEnabled(!stretch);
		holder->setVerticalStretch(stretch);
		holder->attach(item);
		mContent->attach(holder);
	};

	auto createLabel = [&](const std::wstring& str) {
		auto label = std::make_shared<Scene::Label>();
		label->setText(str);
		label->setParseColorTagsEnabled(true);
		label->setFont(mState.font);
		label->setFontSize(mState.font_size);
		return label;
	};

	auto createSprite = [this_index = size_t(this)](const std::string& path) {
		auto sprite = std::make_shared<Scene::Adaptive<Scene::Sprite>>();
		sprite->setTexture(TEXTURE(path));
		sprite->setAdaptStretch(1.0f);
		sprite->setBakingAdaption(true);
		sprite->setBatchGroup(fmt::format("rich_label_icon_{}_{}", this_index, path));
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
			std::wregex tag(fmt::format(L"^<{}>", sky::to_wstring(name)));
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

void SceneHelpers::RichLabel::setTag(const std::string& name, std::function<std::shared_ptr<Scene::Node>()> callback)
{
	mTags[name] = callback;
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

	auto current_frame = FRAME->getFrameCount();

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

	auto rect = IMSCENE->spawn<Smoother<Scene::AutoSized<Scene::ClippableScissor<Scene::Rectangle>>>>(holder, key);
	if (IMSCENE->isFirstCall())
	{
		rect->setAbsoluteRounding(true);
		rect->setRounding(16.0f);
		rect->setColor(Graphics::Color::Black);
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

void SceneHelpers::ImScene::HighlightUnderCursor(Scene::Node& holder, Scene::Node& node)
{
	if (!IsMouseHovered(node))
		return;

	auto bounds = node.getGlobalBounds();

	const auto color = Graphics::Color::Yellow;

	auto rect = IMSCENE->spawn<Shared::SceneHelpers::Smoother<Shared::SceneHelpers::Outlined<Scene::Rectangle>>>(holder);
	if (IMSCENE->isFirstCall())
	{
		rect->setColor({ color, 0.25f });
		rect->getOutlineColor()->setColor({ color, 0.5f });
		rect->setAlpha(0.25f);
	}
	rect->setPosition(holder.unproject(bounds.pos));
	rect->setSize(holder.unproject(bounds.size));
}
