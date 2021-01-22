#pragma once

#include <tinyutf8.hpp>
#include <scene/all.h>
#include <audio/system.h>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
		std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration = 0.5f);

	std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> MakeFastCheckbox(
		const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback = nullptr);

	std::vector<std::shared_ptr<Scene::Node>> MakeFastRadioButtons(std::vector<utf8_string> titles,
		float title_size, int choosed, std::function<void(int)> changeCallback = nullptr);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(float height, const std::vector<std::pair<float/*width*/,
		std::shared_ptr<Scene::Node>>>& items);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>>& items); 

	std::shared_ptr<Scene::Node> MakeVerticalGrid(float width, const std::vector<std::pair<float/*height*/,
		std::shared_ptr<Scene::Node>>>& items);

	std::shared_ptr<Scene::Node> MakeVerticalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>>& items); 

	void RecursiveColorSet(std::shared_ptr<Scene::Node> node, const glm::vec4& color);

	class GrayscaleSprite : public Scene::Sprite
	{
	public:
		GrayscaleSprite();

	protected:
		void draw();

	private:
		static inline std::shared_ptr<Renderer::Shaders::Grayscale> Shader = nullptr;

	public:
		auto getGrayIntensity() const { return mGrayIntensity; }
		void setGrayIntensity(float value) { mGrayIntensity = value; }

	private:
		float mGrayIntensity = 1.0f;
	};

	template <class T> class Button : public Scene::Clickable<T>
	{
	public:
		using Callback = std::function<void()>;

	public:
		virtual void refresh() = 0;

	protected:
		void onClick() override
		{
			Scene::Clickable<T>::onClick();

			auto executeCallback = [](auto callback) { if (callback) callback(); };
			if (isActive())
			{
				executeCallback(mActiveCallback);
				AUDIO->play(mActiveSound);
			}
			else
			{
				executeCallback(mInactiveCallback);
				AUDIO->play(mInactiveSound);
			}
		}

	public:
		auto isActive() const { return mActive; }
		void setActive(bool value) { mActive = value; refresh(); }

		auto getActiveCallback() const { return mActiveCallback; }
		void setActiveCallback(Callback value) { mActiveCallback = value; }

		auto getInactiveCallback() const { return mInactiveCallback; }
		void setInactiveCallback(Callback value) { mInactiveCallback = value; }

		void setActiveSound(std::shared_ptr<Audio::Sound> value) { mActiveSound = value; }
		void setInactiveSound(std::shared_ptr<Audio::Sound> value) { mInactiveSound = value; }

	private:
		bool mActive = true;
		Callback mActiveCallback = nullptr;
		Callback mInactiveCallback = nullptr;
		std::shared_ptr<Audio::Sound> mActiveSound = nullptr;
		std::shared_ptr<Audio::Sound> mInactiveSound = nullptr;
	};

	template <class T> class BouncingButtonBehavior : public T
	{
	public:
		void updateTransform() override
		{
			T::updateTransform();

			auto transform = getTransform();
			transform = glm::translate(transform, { 0.5f * getAbsoluteSize(), 0.0f });
			transform = glm::scale(transform, { mRelativeScale, mRelativeScale, 1.0f });
			transform = glm::translate(transform, { 0.5f * -getAbsoluteSize(), 0.0f });
			setTransform(transform);
		}

	public:
		void onChooseBegin() override
		{	
			T::onChooseBegin();
			
			if (mChooseAnimationProcessing)
				return;

			mChooseAnimationStarted = true;
			runAction(Actions::Factory::MakeSequence(
				Actions::Factory::Execute([this] { 
					mChooseAnimationProcessing = true; 
				}),
				Actions::Factory::Interpolate(1.0f - 0.125f, 0.125f / 4.0f, mRelativeScale),
				Actions::Factory::Execute([this] {
					mChooseAnimationProcessing = false;
				})
			));
		}

		void onChooseEnd() override
		{
			T::onChooseEnd();

			if (!mChooseAnimationStarted)
				return;

			const float Duration = 0.125f / 1.5f;

			mChooseAnimationStarted = false;
			runAction(Actions::Factory::MakeSequence(
				Actions::Factory::Wait(mChooseAnimationProcessing),
				Actions::Factory::Execute([this] { 
					mChooseAnimationProcessing = true; 
				}),
				Actions::Factory::Interpolate(1.125f, Duration / 2.0f, mRelativeScale),
				Actions::Factory::Interpolate(1.0f, Duration / 2.0f, mRelativeScale),
				Actions::Factory::Execute([this] {
					mChooseAnimationProcessing = false;
				})
			));
		}

	private:
		float mRelativeScale = 1.0f;
		bool mChooseAnimationProcessing = false;
		bool mChooseAnimationStarted = false;
	};

	class GrayscaleSpriteButton : public Button<GrayscaleSprite>
	{
	public:
		GrayscaleSpriteButton();

	public:
		void refresh() override;
	};

	class FastButton : public Button<Scene::Rectangle>
	{
	public:
		FastButton();

	public:
		void refresh() override;

	public:
		auto getLabel() { return mLabel; }

	private:
		std::shared_ptr<Scene::Label> mLabel;
	};

	class Progressbar : public Scene::Rectangle
	{
	public:
		Progressbar();

	public:
		void setProgress(float value);
		float getProgress() const;

		auto getProgressContent() const { return mProgressContent; }

	private:
		std::shared_ptr<Scene::Rectangle> mProgressContent;
	};

	// TODO: this can be moved to 'scene' namespace
	// but we should solve the problem with actions in 'shared'.
	// 'scene' does not know anything about 'shared'
	template <class T> class Emitter : public Scene::Node
	{
	public:
		Emitter()
		{
			runAction(Actions::Factory::RepeatInfinite([this]()->Actions::Factory::UAction {
				if (!mRunning)
					return nullptr;

				auto delay = glm::linearRand(mMinDelay, mMaxDelay);
				return Actions::Factory::Delayed(delay, Actions::Factory::Execute([this] {
					if (!mRunning)
						return;

					emit();
				}));
			}));
		}

	public:
		void emit(int count = 1)
		{
			assert(!mHolder.expired());
			assert(count > 0);

			for (int i = 0; i < count - 1; i++)
			{
				emit();
			}
			
			auto holder = mHolder.lock();

			auto particle = createParticle();			
			particle->setColor(mBeginColor);
			particle->setAlpha(0.0f);
			particle->setPosition(holder->unproject(project(getAbsoluteSize() * glm::linearRand(glm::vec2(0.0f), glm::vec2(1.0f)))));
			particle->setScale(mBeginScale);
			particle->setPivot(0.5f);
			particle->setRotation(glm::radians(glm::linearRand(0.0f, 360.0f)));

			auto duration = glm::linearRand(mMinDuration, mMaxDuration);
			auto direction = glm::linearRand(mMinDirection, mMaxDirection);

			particle->runAction(Actions::Factory::MakeSequence(
				Actions::Factory::MakeParallel(
					Actions::Factory::ChangePosition(particle, particle->getPosition() + (direction * mDistance), duration, Easing::CubicOut),
					Actions::Factory::ChangeScale(particle, mEndScale, duration),
					Actions::Factory::ChangeColor(particle, mBeginColor, mEndColor, duration),
					Actions::Factory::ChangeAlpha(particle, mBeginColor.a, mEndColor.a, duration)
				),
				Actions::Factory::Kill(particle)
			));

			holder->attach(particle);
		}

	protected:
		virtual std::shared_ptr<T> createParticle() const = 0;

	public:
		void setHolder(std::weak_ptr<Scene::Node> value) { mHolder = value; }

		bool isRunning() const { return mRunning; }
		void setRunning(bool value) { mRunning = value; }

		auto getMinDelay() const { return mMinDelay; }
		void setMinDelay(float value) { mMinDelay = value; }

		auto getMaxDelay() const { return mMaxDelay; }
		void setMaxDelay(float value) { mMaxDelay = value; }

		void setDelay(float value) { setMinDelay(value); setMaxDelay(value); }

		auto getBeginScale() const { return mBeginScale; }
		void setBeginScale(const glm::vec2& value) { mBeginScale = value; }

		auto getEndScale() const { return mEndScale; }
		void setEndScale(const glm::vec2& value) { mEndScale = value; }

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
		std::weak_ptr<Scene::Node> mHolder;
		bool mRunning = true;
		float mMinDelay = 0.5f;
		float mMaxDelay = 0.5f;
		glm::vec2 mBeginScale = { 1.0f, 1.0f };
		glm::vec2 mEndScale = { 0.0f, 0.0f };
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
	protected:
		std::shared_ptr<Scene::Sprite> createParticle() const override
		{
			auto particle = std::make_shared<Scene::Sprite>();
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
	protected:
		std::shared_ptr<Scene::Rectangle> createParticle() const override
		{
			auto particle = std::make_shared<Scene::Rectangle>();
			particle->setRounding(mRounding);
			particle->setSize(mBeginSize);
			return particle;
		}

	public:
		auto getRounding() const { return mRounding; }
		void setRounding(float value) { mRounding = value; }

		auto getBeginSize() const { return mBeginSize; }
		void setBeginSize(const glm::vec2& value) { mBeginSize = value; }

	private:
		float mRounding = 0.0f;
		glm::vec2 mBeginSize = { 8.0f, 8.0f };
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

			auto scale = mAdaptSize / T::getAbsoluteSize();

			T::setScale(glm::min(scale.x, scale.y));
		}

	public:
		auto getAdaptSize() const { return mAdaptSize; }
		void setAdaptSize(const glm::vec2& value) { mAdaptSize = value; }

	private:
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
	};

	template <typename T> class Backshaded : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	protected:
		void enterDraw() override
		{
			T::enterDraw();

			GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			GRAPHICS->drawRectangle(glm::mat4(1.0f), mBackshadeColor->getColor());
			GRAPHICS->pop();
		}

	public:
		auto getBackshadeColor() const { return mBackshadeColor; }

	private:
		std::shared_ptr<Scene::Color> mBackshadeColor = std::make_shared<Scene::Color>();
	};

	template <typename T> class Outlined : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	protected:
		void leaveDraw() override
		{
			T::leaveDraw();

			auto model = glm::scale(T::getTransform(), { T::getAbsoluteSize(), 1.0f });

			GRAPHICS->drawLineRectangle(model, { 1.0f, 1.0f, 1.0f, 1.0 });
		}
	};

	// automatically stretching to full safe area of the screen
	// should be attached to fullscreen node or scene root
	class Hud : public Scene::Node
	{
	public:
		Hud();

	protected:
		void update() override;
	};
}