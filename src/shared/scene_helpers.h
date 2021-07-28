#pragma once

#include <tinyutf8.hpp>
#include <scene/all.h>
#include <audio/system.h>
#include "scene_manager.h"

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

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items);
	std::shared_ptr<Scene::Node> MakeVerticalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items);
	std::shared_ptr<Scene::Node> MakeGrid(const std::vector<std::vector<std::shared_ptr<Scene::Node>>>& items);

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
		virtual void refresh()
		{
            Scene::Clickable<T>::setClickEnabled(isActive());
		};

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

			auto transform = T::getTransform();
			transform = glm::translate(transform, { 0.5f * T::getAbsoluteSize(), 0.0f });
			transform = glm::scale(transform, { mRelativeScale, mRelativeScale, 1.0f });
			transform = glm::translate(transform, { 0.5f * -T::getAbsoluteSize(), 0.0f });
            T::setTransform(transform);
		}

	public:
		void onChooseBegin() override
		{	
			T::onChooseBegin();
			
			if (mChooseAnimationProcessing)
				return;

			mChooseAnimationStarted = true;
			T::runAction(Actions::Collection::MakeSequence(
				Actions::Collection::Execute([this] {
					mChooseAnimationProcessing = true; 
				}),
				Actions::Collection::Interpolate(1.0f - 0.125f, 0.125f / 4.0f, mRelativeScale),
				Actions::Collection::Execute([this] {
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
			T::runAction(Actions::Collection::MakeSequence(
				Actions::Collection::Wait(mChooseAnimationProcessing),
				Actions::Collection::Execute([this] {
					mChooseAnimationProcessing = true; 
				}),
				Actions::Collection::Interpolate(1.125f, Duration / 2.0f, mRelativeScale),
				Actions::Collection::Interpolate(1.0f, Duration / 2.0f, mRelativeScale),
				Actions::Collection::Execute([this] {
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

	class RectangleButton : public Button<Scene::Rectangle>
	{
	public:
		RectangleButton();

	public:
		void refresh() override;

	public:
		void onChooseBegin() override;
		void onChooseEnd() override;

	public:
		auto getLabel() { return mLabel; }

		auto isHighlightEnabled() const { return mHighlightEnabled;  }
		void setHighlightEnabled(bool value) { mHighlightEnabled = value; }

		auto getActiveColor() const { return mActiveColor; }
		void setActiveColor(const glm::vec4& value) { mActiveColor = value; }

		auto getInactiveColor() const { return mInactiveColor; }
		void setInactiveColor(const glm::vec4& value) { mInactiveColor = value; }

		auto getHighlightColor() const { return mHighlightColor; }
		void setHighlightColor(const glm::vec4& value) { mHighlightColor = value; }

	private:
		std::shared_ptr<Scene::Label> mLabel;
		bool mHighlightEnabled = true;
		glm::vec4 mActiveColor = { 1.0f, 1.0f, 1.0f, 0.33f };
		glm::vec4 mInactiveColor = { 1.0f, 1.0f, 1.0f, 0.125f };
		glm::vec4 mHighlightColor = { 1.0f, 1.0f, 1.0f, 0.66f };
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
	class SafeArea : public Scene::Node
	{
	public:
		SafeArea();

	protected:
		void update(Clock::Duration dTime) override;
	};

	class VerticalScrollbar : public Scene::Rectangle,
		public std::enable_shared_from_this<VerticalScrollbar>
	{
	private:
		const float BarAlpha = 0.125f;
		const float IndicatorAlpha = 0.5f;

	public:
		VerticalScrollbar();

	public:
		void update(Clock::Duration dTime) override;

	public:
		void setScrollbox(std::weak_ptr<Scene::Scrollbox> value) { mScrollbox = value; }

	private:
		std::weak_ptr<Scene::Scrollbox> mScrollbox;
		std::shared_ptr<Scene::Rectangle> mIndicator;
		float mPrevScrollPosition = 0.0f;
		Clock::TimePoint mScrollMoveTime = Clock::Now();
		bool mAlphaAnimating = false;
		bool mHidden = false;
	};

	class StandardScreen : public SceneManager::Screen
	{
	public:
		StandardScreen();

	public:
		auto getContent() const { return mContent; }

	protected:
		void onEnterBegin() override;
		void onEnterEnd() override;
		void onLeaveBegin() override;
		void onLeaveEnd() override;
		void onWindowAppearing() override;
		void onWindowDisappearing() override;

	protected:
		std::unique_ptr<Actions::Action> createEnterAction() override;
		std::unique_ptr<Actions::Action> createLeaveAction() override;

	private:
		std::shared_ptr<Scene::Node> mContent;
		std::shared_ptr<Scene::Rectangle> mFadeRectangle;
	};

	class StandardWindow : public Scene::Clickable<Backshaded<Shared::SceneManager::Window>>
	{
	public:
		StandardWindow();

	public:
		void onOpenEnd() override;
		void onCloseBegin() override;

	public:
		std::unique_ptr<Actions::Action> createOpenAction() override;
		std::unique_ptr<Actions::Action> createCloseAction() override;

	public:
		auto getContent() { return mContent; }

		auto getCloseOnMissclick() const { return mCloseOnMissclick; }
		void setCloseOnMissclick(bool value) { mCloseOnMissclick = value; }

	private:
		std::shared_ptr<Scene::Node> mContent;
		bool mCloseOnMissclick = true;
	};
}
