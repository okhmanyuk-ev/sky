#include "scene_helpers.h"
#include <shared/cache_system.h>
#include <tiny_obj_loader.h>
#include <sstream>

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

std::shared_ptr<Scene::Node> SceneHelpers::MakeHorizontalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items)
{
	auto holder = std::make_shared<Scene::Node>();
	float stretch_x = 1.0f / (float)items.size();
	float anchor_x = 0.0f;
	for (auto item : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setStretch({ stretch_x, 1.0f });
		cell->setHorizontalAnchor(anchor_x);
		holder->attach(cell);
		cell->attach(item);
		anchor_x += stretch_x;
	}
	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items)
{
	auto holder = std::make_shared<Scene::Node>();
	float stretch_y = 1.0f / (float)items.size();
	float anchor_y = 0.0f;
	for (auto item : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setStretch({ 1.0f, stretch_y });
		cell->setVerticalAnchor(anchor_y);
		holder->attach(cell);
		cell->attach(item);
		anchor_y += stretch_y;
	}
	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeGrid(const std::vector<std::vector<std::shared_ptr<Scene::Node>>>& items)
{
	std::vector<std::shared_ptr<Scene::Node>> horz_grids;
	for (auto item : items)
	{
		auto horz_grid = MakeHorizontalGrid(item);
		horz_grid->setStretch(1.0f);
		horz_grids.push_back(horz_grid);
	}
	return MakeVerticalGrid(horz_grids);
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
	mLabel = std::make_shared<Scene::Label>();
	mLabel->setAnchor(0.5f);
	mLabel->setPivot(0.5f);
	attach(mLabel);

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
	mProgressContent->setHorizontalStretch(value);
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
	mIndicator->setSize({ 0.0f, 32.0f });
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

SceneHelpers::StandardScreen::StandardScreen()
{
	setEnabled(false);
	setInteractions(false);
	setStretch(1.0f);
	setAnchor(0.5f);
	setPivot(0.5f);
	getFrontshadeColor()->setColor(Graphics::Color::Black);
}

void SceneHelpers::StandardScreen::onEnterBegin()
{
	setEnabled(true);
}

void SceneHelpers::StandardScreen::onEnterEnd()
{
	setInteractions(true);
}

void SceneHelpers::StandardScreen::onLeaveBegin()
{
	setInteractions(false);
}

void SceneHelpers::StandardScreen::onLeaveEnd()
{
	setEnabled(false);
}

void SceneHelpers::StandardScreen::onWindowAppearing()
{
	setInteractions(false);
}

void SceneHelpers::StandardScreen::onWindowDisappearing()
{
	setInteractions(true);
}

std::unique_ptr<Actions::Action> SceneHelpers::StandardScreen::createEnterAction()
{
	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitOneFrame(),
		Actions::Collection::ChangeAlpha(getFrontshadeColor(), 0.0f, 0.5f)
	);
};

std::unique_ptr<Actions::Action> SceneHelpers::StandardScreen::createLeaveAction()
{
	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitOneFrame(),
		Actions::Collection::ChangeAlpha(getFrontshadeColor(), 1.0f, 0.5f)
	);
};

// standard window

SceneHelpers::StandardWindow::StandardWindow(const std::set<Flag> flags) : mFlags(flags)
{
	setStretch(1.0f);
	setClickCallback([this] {
		if (!mCloseOnMissclick)
			return;

		if (getState() != State::Opened)
			return;

		SCENE_MANAGER->popWindow();
	});

	mContent = std::make_shared<Scene::Node>();
	mContent->setStretch(1.0f);
	mContent->setAnchor({ 0.5f, -0.5f });
	mContent->setPivot({ 0.5f, 0.5f });
	mContent->setInteractions(false);
	attach(mContent);

	getBackshadeColor()->setColor({ Graphics::Color::Black, 0.0f });

	if (flags.contains(Flag::Blur))
	{
		mBlur = std::make_shared<Scene::BlurredGlass>();
		mBlur->setStretch(1.0f);
		mBlur->setBlurIntensity(0.0f);
		attach(mBlur, Scene::Node::AttachDirection::Front);
	}

	if (flags.contains(Flag::Gray))
	{
		mGray = std::make_shared<Scene::GrayscaledGlass>();
		mGray->setStretch(1.0f);
		mGray->setGrayscaleIntensity(0.0f);
		attach(mGray, Scene::Node::AttachDirection::Front);
	}
}

void SceneHelpers::StandardWindow::onOpenEnd()
{
	mContent->setInteractions(true);
}

void SceneHelpers::StandardWindow::onCloseBegin()
{
	mContent->setInteractions(false);
}

std::unique_ptr<Actions::Action> SceneHelpers::StandardWindow::createOpenAction()
{
	const float Duration = 0.5f;

	auto features = Actions::Collection::MakeParallel();

	if (mFlags.contains(Flag::Fade))
	{
		features->add(Actions::Collection::ChangeAlpha(getBackshadeColor(), 0.5f, Duration, Easing::CubicOut));
	}

	if (mFlags.contains(Flag::Blur))
	{
		features->add(Actions::Collection::ChangeBlurIntensity(mBlur, 1.0f, Duration, Easing::CubicOut));
		features->add(Actions::Collection::ChangeColor(mBlur, glm::vec3(1.0f + (0.125f / 2.0f)), Duration, Easing::CubicOut));
	}

	if (mFlags.contains(Flag::Gray))
	{
		features->add(Actions::Collection::ChangeGrayscaleIntensity(mGray, 1.0f, Duration, Easing::CubicOut));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitOneFrame(),
		Actions::Collection::MakeParallel(
			Actions::Collection::ChangeVerticalAnchor(mContent, 0.5f, 0.5f, Easing::CubicOut),
			std::move(features)
		)
	);
};

std::unique_ptr<Actions::Action> SceneHelpers::StandardWindow::createCloseAction()
{
	const float Duration = 0.5f;

	auto features = Actions::Collection::MakeParallel();

	if (mFlags.contains(Flag::Fade))
	{
		features->add(Actions::Collection::ChangeAlpha(getBackshadeColor(), 0.0f, Duration, Easing::CubicIn));
	}

	if (mFlags.contains(Flag::Blur))
	{
		features->add(Actions::Collection::ChangeBlurIntensity(mBlur, 0.0f, Duration, Easing::CubicIn));
		features->add(Actions::Collection::ChangeColor(mBlur, glm::vec3(1.0f), Duration, Easing::CubicIn));
	}

	if (mFlags.contains(Flag::Gray))
	{
		features->add(Actions::Collection::ChangeGrayscaleIntensity(mGray, 0.0f, Duration, Easing::CubicIn));
	}

	return Actions::Collection::MakeSequence(
		Actions::Collection::WaitOneFrame(),
		Actions::Collection::MakeParallel(
			Actions::Collection::ChangeVerticalAnchor(mContent, -0.5f, 0.5f, Easing::CubicIn),
			std::move(features)
		)
	);
};

// blurred glass demo

SceneHelpers::BlurredGlassDemo::BlurredGlassDemo()
{
	setClickCallback([this] {
		runAction(Actions::Collection::Kill(shared_from_this()));
	});

	auto blur = std::make_shared<Shared::SceneHelpers::Outlined<Scene::BlurredGlass>>();
	blur->setStretch(1.0f);
	attach(blur);

	auto rect = std::make_shared<Scene::Rectangle>();
	rect->setStretch({ 1.0f, 0.0f });
	rect->setHeight(8.0f);
	rect->setAnchor({ 0.5f, 1.0f });
	rect->setPivot({ 0.5f, 0.0f });
	rect->setY(4.0f);
	rect->setAlpha(0.25f);
	blur->attach(rect);

	auto slider = std::make_shared<Scene::Rectangle>();
	slider->setPivot(0.5f);
	slider->setAnchor(0.5f);
	slider->setStretch({ 0.0f, 1.0f });
	slider->setWidth(8.0f);

	auto scrollbox = std::make_shared<Scene::Scrollbox>();
	scrollbox->setInertiaEnabled(false);
	scrollbox->setStretch(1.0f);
	scrollbox->getBounding()->setStretch(1.0f);
	scrollbox->getBounding()->setAnchor(0.5f);
	scrollbox->getBounding()->setPivot(0.5f);
	scrollbox->getContent()->setStretch({ 2.0f, 1.0f });
	scrollbox->getContent()->attach(slider);
	rect->attach(scrollbox);

	auto label = std::make_shared<Scene::Label>();
	label->setPosition({ 2.0f, 2.0f });
	label->setFontSize(12.0f);
	blur->attach(label);

	runAction(Actions::Collection::ExecuteInfinite([scrollbox, slider, blur, label] {
		auto h_pos = scrollbox->getHorizontalScrollPosition();
		blur->setBlurIntensity(1.0f - h_pos);
		slider->setHorizontalPivot(1.0f - h_pos);
		label->setText(fmt::format("{:.{}f}", blur->getBlurIntensity(), 2));
	}));
}

// 3d

std::vector<std::shared_ptr<Scene3D::Model>> SceneHelpers::MakeModelsFromObj(const std::string& path_to_folder, const std::string& name_without_extension)
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
