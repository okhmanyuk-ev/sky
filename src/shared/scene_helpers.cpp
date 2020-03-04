#include "scene_helpers.h"
#include <scene/actionable.h>
#include <shared/action_helpers.h>

using namespace Shared;

std::shared_ptr<Scene::Label> SceneHelpers::MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
	std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration)
{
	auto label = std::make_shared<Scene::Actionable<Scene::Label>>();
	label->setFont(FONT("default"));
	label->setFontSize(text_size);
	label->setText(text);
	label->setPosition(holder->unproject(target->project({ target->getSize() / 2.0f })));
	label->setPivot({ 0.5f, 0.5f });
	label->setAlpha(0.0f);
	label->runAction(ActionHelpers::MakeSequence(
		ActionHelpers::Show(label, 0.5f),
		ActionHelpers::ChangePositionByDirection(label, { 0.0f, -1.0f }, 64.0f, move_duration),
		ActionHelpers::Hide(label, 0.5f),
		ActionHelpers::Kill(label)
	));
	holder->attach(label);
	return label;
}

std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> SceneHelpers::MakeFastCheckbox(
	const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback)
{
	auto holder = std::make_shared<Scene::Clickable<Scene::Node>>();

	auto outer_rect = std::make_shared<Scene::Actionable<Scene::Rectangle>>();
	outer_rect->setAlpha(0.33f);
	outer_rect->setVerticalStretch(1.0f);
	outer_rect->setMargin({ 8.0f, 8.0f });
	outer_rect->setAnchor({ 0.0f, 0.5f });
	outer_rect->setPivot({ 0.0f, 0.5f });
	holder->attach(outer_rect);

	outer_rect->runAction(ActionHelpers::ExecuteInfinite([outer_rect] {
		outer_rect->setWidth(outer_rect->getHeight());
	}));

	auto inner_rect = std::make_shared<Scene::Rectangle>();
	inner_rect->setAnchor({ 0.5f, 0.5f });
	inner_rect->setPivot({ 0.5f, 0.5f });
	inner_rect->setStretch({ 0.66f, 0.66f });
	inner_rect->setAlpha(0.66f);
	inner_rect->setEnabled(checked);
	outer_rect->attach(inner_rect);

	auto label = std::make_shared<Scene::Label>();
	label->setFont(FONT("default"));
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
	holder->setChooseCallback([outer_rect] { outer_rect->setAlpha(0.66f); });
	holder->setCancelChooseCallback([outer_rect] { outer_rect->setAlpha(0.33f); });

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
	const std::vector<std::pair<float/*width*/, std::shared_ptr<Scene::Node>>> items)
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
	const std::vector<std::shared_ptr<Scene::Node>> items)
{
	auto items_width = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_width.push_back({ cell_size.x, item });
	}

	return MakeHorizontalGrid(cell_size.y, items_width);
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(float width,
	const std::vector<std::pair<float/*height*/, std::shared_ptr<Scene::Node>>> items)
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
	const std::vector<std::shared_ptr<Scene::Node>> items)
{
	auto items_height = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_height.push_back({ cell_size.y, item });
	}

	return MakeVerticalGrid(cell_size.x, items_height);
}

SceneHelpers::FastButton::FastButton()
{
	setChooseCallback([this] { setAlpha(0.66f); });
	setCancelChooseCallback([this] { setAlpha(0.33f); });

	mLabel = std::make_shared<Scene::Label>();
	mLabel->setFont(FONT("default"));
	mLabel->setAnchor({ 0.5f, 0.5f });
	mLabel->setPivot({ 0.5f, 0.5f });
	attach(mLabel);

	setButtonActive(true);
}

void SceneHelpers::FastButton::setButtonActive(bool value)
{
	setAlpha(value ? 0.33f : 0.125f);
	setClickEnabled(value);
	mButtonActive = value;
}

SceneHelpers::Progressbar::Progressbar()
{
	setAlpha(0.33f);
	
	mProgress = std::make_shared<Scene::Rectangle>();
	mProgress->setAnchor({ 0.0f, 0.5f });
	mProgress->setPivot({ 0.0f, 0.5f });
	mProgress->setVerticalStretch(1.0f);
	mProgress->setAlpha(0.66f);
	attach(mProgress);
}

void SceneHelpers::Progressbar::setProgress(float value)
{
	mProgress->setHorizontalStretch(value);
}

float SceneHelpers::Progressbar::getProgress() const
{
	return mProgress->getHorizontalStretch();
}

SceneHelpers::SplashScene::SplashScene(std::shared_ptr<Renderer::Texture> logo)
{
	auto root = getRoot();

	auto image = std::make_shared<::Scene::Sprite>();
	image->setTexture(logo);
	image->setAnchor({ 0.5f, 0.5f });
	image->setPivot({ 0.5f, 0.5f });
	image->setSampler(Renderer::Sampler::Linear);
	image->setSize({ 286.0f, 286.0f });
	root->attach(image);

	mProgressbar = std::make_shared<Progressbar>();
	mProgressbar->setSize({ 224.0f, 4.0f });
	mProgressbar->setPivot({ 0.5f, 0.5f });
	mProgressbar->setAnchor({ 0.5f, 0.8f });
	root->attach(mProgressbar);
}

void SceneHelpers::SplashScene::updateProgress(float value)
{
	mProgressbar->setProgress(value);
}

SceneHelpers::Emitter::Emitter(std::weak_ptr<Scene::Node> holder) : mHolder(holder)
{
	runAction(Shared::ActionHelpers::RepeatInfinite([this] {
		auto delay = glm::linearRand(mMinDelay, mMaxDelay);
		return Shared::ActionHelpers::Delayed(delay, Shared::ActionHelpers::Execute([this] {
			emit();
		}));
	}));
}

void SceneHelpers::Emitter::emit()
{
	assert(mParticleTexture != nullptr);

	auto holder = mHolder.lock();

	auto particle = std::make_shared<Scene::Actionable<Scene::Sprite>>();
	particle->setTexture(mParticleTexture);
	particle->setPosition(holder->unproject(project(getSize() * glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f)))));
	particle->setSize(mParticleSize);
	particle->setPivot({ 0.5f, 0.5f });
	particle->setRotation(glm::radians(glm::linearRand(0.0f, 360.0f)));
	particle->setColor(mBeginColor);
	particle->setAlpha(0.0f);

	auto direction = glm::linearRand(mMinDirection, mMaxDirection);
	
	particle->runAction(Shared::ActionHelpers::MakeSequence(
		Shared::ActionHelpers::MakeParallel(
			Shared::ActionHelpers::ChangePosition(particle, particle->getPosition() + (direction * mDistance), mDuration, Common::Easing::CubicOut),
			Shared::ActionHelpers::ChangeScale(particle, { 0.0f, 0.0f }, mDuration),
			Shared::ActionHelpers::ChangeColor(particle, mBeginColor, mEndColor, mDuration),
			Shared::ActionHelpers::ChangeAlpha(particle, mBeginColor.a, mEndColor.a, mDuration)
		),
		Shared::ActionHelpers::Kill(particle)
	));

	holder->attach(particle);
}