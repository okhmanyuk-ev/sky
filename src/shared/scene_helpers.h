#pragma once

#include <shared/cache_system.h>
#include <tinyutf8.hpp>
#include <scene/label.h>
#include <scene/clickable.h>
#include <scene/rectangle.h>
#include <scene/actionable.h>
#include <scene/debuggable.h>
#include <scene/scene.h>
#include <scene/sprite.h>
#include <shared/action_helpers.h>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
		std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration = 0.5f);

	std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> MakeFastCheckbox(
		const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback = nullptr);

	std::vector<std::shared_ptr<Scene::Node>> MakeFastRadioButtons(std::vector<utf8_string> titles,
		float title_size, int choosed, std::function<void(int)> changeCallback = nullptr);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(float height, const std::vector<std::pair<float/*width*/,
		std::shared_ptr<Scene::Node>>> items);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>> items); // TODO: std::span<..>

	std::shared_ptr<Scene::Node> MakeVerticalGrid(float width, const std::vector<std::pair<float/*height*/,
		std::shared_ptr<Scene::Node>>> items);

	std::shared_ptr<Scene::Node> MakeVerticalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>> items); // TODO: std::span<..>

	class FastButton : public Scene::Clickable<Scene::Rectangle>
	{
	public:
		FastButton();

	public:
		void setButtonActive(bool value);

		auto getLabel() { return mLabel; }

	private:
		bool mButtonActive = true;
		std::shared_ptr<Scene::Label> mLabel;
	};

	class Progressbar : public Scene::Rectangle
	{
	public:
		Progressbar();

	public:
		void setProgress(float value);
		float getProgress() const;

		auto getProgressColor() const { return mProgress->getColor(); }
		void setProgressColor(const glm::vec4& value) { mProgress->setColor(value); }
		void setProgressColor(const glm::vec3& value) { mProgress->setColor(value); }

	private:
		std::shared_ptr<Scene::Rectangle> mProgress;
	};

	class SplashScene : public Scene::Scene
	{
	public:
		SplashScene(std::shared_ptr<Renderer::Texture> logo);

	public:
		void updateProgress(float value);

	private:
		std::shared_ptr<Progressbar> mProgressbar;
	};

	// TODO: this can be moved to 'scene' namespace
	// but we should solve the problem with actions in 'shared'.
	// 'scene' does not know anything about 'shared'
	template <class T> class Emitter : public Scene::Actionable<Scene::Node>
	{
	public:
		Emitter(std::weak_ptr<Scene::Node> holder) : mHolder(holder)
		{
			runAction(Shared::ActionHelpers::RepeatInfinite([this]()->Shared::ActionHelpers::Action {
				if (!mRunning)
					return nullptr;

				auto delay = glm::linearRand(mMinDelay, mMaxDelay);
				return Shared::ActionHelpers::Delayed(delay, Shared::ActionHelpers::Execute([this] {
					if (!mRunning)
						return;

					emit();
				}));
			}));
		}

	public:
		void emit(int count = 1)
		{
			assert(count > 0);

			for (int i = 0; i < count - 1; i++)
			{
				emit();
			}
			
			auto holder = mHolder.lock();

			auto particle = createParticle();			
			particle->setColor(mBeginColor);
			particle->setAlpha(0.0f);
			particle->setPosition(holder->unproject(project(getSize() * glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f)))));
			particle->setSize(mBeginSize);
			particle->setPivot({ 0.5f, 0.5f });
			particle->setRotation(glm::radians(glm::linearRand(0.0f, 360.0f)));

			auto duration = glm::linearRand(mMinDuration, mMaxDuration);
			auto direction = glm::linearRand(mMinDirection, mMaxDirection);

			particle->runAction(Shared::ActionHelpers::MakeSequence(
				Shared::ActionHelpers::MakeParallel(
					Shared::ActionHelpers::ChangePosition(particle, particle->getPosition() + (direction * mDistance), duration, Common::Easing::CubicOut),
					Shared::ActionHelpers::ChangeSize(particle, mEndSize, duration),
					Shared::ActionHelpers::ChangeColor(particle, mBeginColor, mEndColor, duration),
					Shared::ActionHelpers::ChangeAlpha(particle, mBeginColor.a, mEndColor.a, duration)
				),
				Shared::ActionHelpers::Kill(particle)
			));

			holder->attach(particle);
		}

	protected:
		virtual std::shared_ptr<Scene::Actionable<T>> createParticle() const = 0;

	private:
		std::weak_ptr<Scene::Node> mHolder;

	public:
		bool isRunning() const { return mRunning; }
		void setRunning(bool value) { mRunning = value; }

		auto getMinDelay() const { return mMinDelay; }
		void setMinDelay(float value) { mMinDelay = value; }

		auto getMaxDelay() const { return mMaxDelay; }
		void setMaxDelay(float value) { mMaxDelay = value; }

		void setDelay(float value) { setMinDelay(value); setMaxDelay(value); }

		auto getBeginSize() const { return mBeginSize; }
		void setBeginSize(const glm::vec2& value) { mBeginSize = value; }

		auto getEndSize() const { return mEndSize; }
		void setEndSize(const glm::vec2& value) { mEndSize = value; }

		auto getDistance() const { return mDistance; }
		void setDistance(float value) { mDistance = value; }

		auto getMinDuration() const { return mMinDuration; }
		void setMinDuration(float value) { mMinDuration = value; }

		auto getMaxDuration() const { return mMaxDuration; }
		void setMaxDuration(float value) { mMaxDuration = value; }

		void setDuration(float value) { setMinDuration(value); setMaxDuration(value); }

		auto getBeginColor() const { return mBeginColor; }
		void setBeginColor(const glm::vec4& value) { mBeginColor = value; }

		auto getEndColor() const { return mEndColor; }
		void setEndColor(const glm::vec4& value) { mEndColor = value; }

		auto getMinDirection() const { return mMinDirection; }
		void setMinDirection(const glm::vec2& value) { mMinDirection = value; }

		auto getMaxDirection() const { return mMaxDirection; }
		void setMaxDirection(const glm::vec2& value) { mMaxDirection = value; }

		void setDirection(const glm::vec2& value) { setMinDirection(value); setMaxDirection(value); }

	private:
		bool mRunning = true;
		float mMinDelay = 0.5f;
		float mMaxDelay = 0.5f;
		glm::vec2 mBeginSize = { 8.0f, 8.0f };
		glm::vec2 mEndSize = { 0.0f, 0.0f };
		float mDistance = 32.0f;
		float mMinDuration = 1.0f;
		float mMaxDuration = 1.0f;
		glm::vec4 mBeginColor = { Graphics::Color::White, 1.0f };
		glm::vec4 mEndColor = { Graphics::Color::White, 0.0f };
		glm::vec2 mMinDirection = { -1.0f, -1.0f };
		glm::vec2 mMaxDirection = { 1.0f, 1.0f };
	};

	class SpriteEmitter : public Emitter<Scene::Sprite>, public Scene::Blend, public Scene::Sampler
	{
	public:
		SpriteEmitter(std::weak_ptr<Scene::Node> holder) : Emitter(holder) { }

	protected:
		std::shared_ptr<Scene::Actionable<Scene::Sprite>> createParticle() const override
		{
			auto particle = std::make_shared<Scene::Actionable<Scene::Sprite>>();
			particle->setTexture(mTexture);
			particle->setSampler(getSampler());
			particle->setBlendMode(getBlendMode());
			return particle;
		}

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
	};

	class RectangleEmitter : public Emitter<Scene::Rectangle>
	{
	public:
		RectangleEmitter(std::weak_ptr<Scene::Node> holder) : Emitter(holder) { }
	
	protected:
		std::shared_ptr<Scene::Actionable<Scene::Rectangle>> createParticle() const override
		{
			auto particle = std::make_shared<Scene::Actionable<Scene::Rectangle>>();
			return particle;
		}
	};

	// TODO: can be moved to 'scene'
	template <typename T> class Adaptive : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	protected:
		void update() override
		{
			adapt();
			T::update();
		}

	private:
		void adapt()
		{
			if (mAdaptSize.x <= 0.0f)
				return;

			if (mAdaptSize.y <= 0.0f)
				return;

			auto scale = mAdaptSize / T::getSize();

			T::setScale(glm::min(scale.x, scale.y));
		}

	public:
		auto getAdaptSize() const { return mAdaptSize; }
		void setAdaptSize(const glm::vec2& value) { mAdaptSize = value; }

	private:
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
	};
}