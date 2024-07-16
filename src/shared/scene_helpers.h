#pragma once

#include <tinyutf8/tinyutf8.h>
#include <scene/all.h>
#include <scene3d/all.h>
#include <audio/system.h>
#include "scene_manager.h"
#include <common/helpers.h>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
		std::shared_ptr<Scene::Node> target, const tiny_utf8::string& text, float text_size, float move_duration = 0.5f);

	std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> MakeFastCheckbox(
		const tiny_utf8::string& title, float title_size, bool checked, std::function<void(bool)> changeCallback = nullptr);

	std::vector<std::shared_ptr<Scene::Node>> MakeFastRadioButtons(std::vector<tiny_utf8::string> titles,
		float title_size, int choosed, std::function<void(int)> changeCallback = nullptr);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(float height, const std::vector<std::pair<float/*width*/,
		std::shared_ptr<Scene::Node>>>& items);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>>& items); 

	std::shared_ptr<Scene::Node> MakeVerticalGrid(std::optional<float> width, const std::vector<std::pair<float/*height*/,
		std::shared_ptr<Scene::Node>>>& items);

	std::shared_ptr<Scene::Node> MakeVerticalGrid(std::optional<float> width, float height,
		const std::vector<std::shared_ptr<Scene::Node>>& items);

	std::shared_ptr<Scene::Node> MakeVerticalGrid(glm::vec2 size,
		const std::vector<std::shared_ptr<Scene::Node>>& items);

	struct GridPart
	{
		float part = 1.0f;
		std::shared_ptr<Scene::Node> node;
	};
	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const std::vector<GridPart>& items);
	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items);
	std::shared_ptr<Scene::Node> MakeVerticalGrid(const std::vector<std::shared_ptr<Scene::Node>>& items);
	std::shared_ptr<Scene::Node> MakeGrid(const std::vector<std::vector<std::shared_ptr<Scene::Node>>>& items);

	std::shared_ptr<Scene::Node> MakeStretchedToContentVerticalGrid(
		const std::vector<std::shared_ptr<Scene::Node>>& items);

	void RecursiveColorSet(std::shared_ptr<Scene::Node> node, const glm::vec4& color);

	class GrayscaleSprite : public Scene::Sprite
	{
	protected:
		void draw() override;

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
				sky::PlaySound(mActiveSound);				
				PLATFORM->haptic(Platform::System::HapticType::Medium);
			}
			else
			{
				executeCallback(mInactiveCallback);
				sky::PlaySound(mInactiveSound);
			}
		}

		void onChooseBegin() override
		{
			Scene::Clickable<T>::onChooseBegin();
			PLATFORM->haptic(Platform::System::HapticType::Low);
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
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");
	public:
		void updateTransform() override
		{
			T::updateTransform();

			auto offset = 0.5f * T::getAbsoluteSize();

			auto transform = T::getTransform();
			transform = glm::translate(transform, { offset, 0.0f });
			transform = glm::scale(transform, { mRelativeScale, mRelativeScale, 1.0f });
			transform = glm::translate(transform, { -offset, 0.0f });
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

	class SpriteButton : public Button<GrayscaleSprite>
	{
	public:
		SpriteButton();

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
		void setActiveColor(const glm::vec3& value) { setActiveColor({ value.x, value.y, value.z, mActiveColor.a }); }

		auto getInactiveColor() const { return mInactiveColor; }
		void setInactiveColor(const glm::vec4& value) { mInactiveColor = value; }
		void setInactiveColor(const glm::vec3& value) { setInactiveColor({ value.x, value.y, value.z, mInactiveColor.a }); }

		auto getHighlightColor() const { return mHighlightColor; }
		void setHighlightColor(const glm::vec4& value) { mHighlightColor = value; }
		void setHighlightColor(const glm::vec3& value) { setHighlightColor({ value.x, value.y, value.z, mHighlightColor.a }); }

	private:
		std::shared_ptr<Scene::Adaptive<Scene::Label>> mLabel;
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

			if (mBackshadeColor->getAlpha() <= 0.0f)
				return;

			GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			GRAPHICS->drawRectangle(nullptr, mBackshadeColor->getColor());
			GRAPHICS->pop();
		}

	public:
		auto getBackshadeColor() const { return mBackshadeColor; }

	private:
		std::shared_ptr<Scene::Color> mBackshadeColor = std::make_shared<Scene::Color>();
	};

	template <typename T> class Frontshaded : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	protected:
		void leaveDraw() override
		{
			T::leaveDraw();

			if (mFrontshadeColor->getAlpha() <= 0.0f)
				return;

			GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			GRAPHICS->drawRectangle(nullptr, mFrontshadeColor->getColor());
			GRAPHICS->pop();
		}

	public:
		auto getFrontshadeColor() const { return mFrontshadeColor; }

	private:
		std::shared_ptr<Scene::Color> mFrontshadeColor = std::make_shared<Scene::Color>();
	};

	template <typename T> class Outlined : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	protected:
		void leaveDraw() override
		{
			T::leaveDraw();

			if (!mOutlined)
				return;

			auto model = glm::scale(T::getTransform(), { T::getAbsoluteSize(), 1.0f });
			
			GRAPHICS->pushModelMatrix(model);
			GRAPHICS->drawLineRectangle();
			GRAPHICS->pop();
		}

	public:
		bool isOutlined() const { return mOutlined; }
		void setOutlined(bool value) { mOutlined = value; }

	private:
		bool mOutlined = true;
	};

	template <typename T> class MovableByHand : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	public:
		MovableByHand()
		{
			T::setTouchable(true);
		}

	protected:
		void touch(Scene::Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			auto fixed_pos = pos;
			
			if (type != Scene::Node::Touch::Begin)
				T::setPosition(T::getPosition() + T::unproject(fixed_pos) - T::unproject(mPrevPosition));

			mPrevPosition = fixed_pos;
		}

	private:
		glm::vec2 mPrevPosition = { 0.0f, 0.0f };
	};

	template <class T> class KillableByClick : public Scene::Clickable<T>,
		public std::enable_shared_from_this<KillableByClick<T>>
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	public:
		KillableByClick()
		{
			Scene::Clickable<T>::setClickCallback([this] {
				Scene::Clickable<T>::runAction(Actions::Collection::Kill(KillableByClick<T>::shared_from_this()));
			});
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

	class StandardScreen : public Scene::RenderLayer<SceneManager::Screen>
	{
	public:
		enum class Effect
		{
			Alpha,
			Blur,
			Scale,
			WindowAppearingScale
		};

	public:
		static const glm::vec2 inline StartScale = { 0.95f, 0.95f };

	public:
		StandardScreen(const std::set<Effect>& effects = { Effect::Alpha, Effect::Scale, Effect::WindowAppearingScale });
		
	protected:
		void onEnterBegin() override;
		void onEnterEnd() override;
		void onLeaveBegin() override;
		void onLeaveEnd() override;
		void onWindowAppearingBegin() override;
		void onWindowDisappearingBegin() override;
		void onWindowDisappearingEnd() override;

	protected:
		std::unique_ptr<Actions::Action> createEnterAction() override;
		std::unique_ptr<Actions::Action> createLeaveAction() override;

	public:
		auto getContent() const { return mContent; }

	private:
		std::set<Effect> mEffects;
		std::shared_ptr<Scene::Node> mContent;
		std::shared_ptr<Scene::BlurredGlass> mBlur;
	};

	class StandardWindow : public Scene::Clickable<Backshaded<Shared::SceneManager::Window>>
	{
	public:
		enum class BackgroundEffect
		{
			Blur,
			Fade,
			Gray
		};

		enum class ContentEffect 
		{
			Anchor,
			Blur,
			Alpha,
			Scale
		};

	public:
		static const glm::vec2 inline StartContentAnchor = { 0.5f, -0.5f };
		static const glm::vec2 inline StartContentScale = { 0.75f, 0.75f };
		static const auto inline StartContentBlur = 1.0f;		

	public:
		StandardWindow(const std::set<BackgroundEffect> background_effect = { BackgroundEffect::Fade }, 
			const std::set<ContentEffect> content_effect = { ContentEffect::Alpha, ContentEffect::Anchor });

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
		std::shared_ptr<Scene::RenderLayer<Scene::Node>> mContentHolder;
		std::shared_ptr<Scene::Node> mContent;
		std::shared_ptr<Scene::BlurredGlass> mContentBlur;
		std::shared_ptr<Scene::BlurredGlass> mBlur;
		std::shared_ptr<Scene::GrayscaledGlass> mGray;
		bool mCloseOnMissclick = true;
		std::set<BackgroundEffect> mBackgroundEffect;
		std::set<ContentEffect> mContentEffect;
	};

	class BlurredGlassDemo : public KillableByClick<MovableByHand<Outlined<Scene::BlurredGlass>>>
	{
	public:
		BlurredGlassDemo();
	};

	class Shockwave : public Scene::Glass
	{
	public:
		static std::shared_ptr<Shockwave> MakeAnimated(float duration = 1.0f);

	public:
		Shockwave();

	public:
		void draw() override;

	public:
		auto getShockwaveSize() const { return mShockwaveSize; }
		void setShockwaveSize(float value) { mShockwaveSize = value; }

		auto getShockwaveThickness() const { return mShockwaveThickness; }
		void setShockwaveThickness(float value) { mShockwaveThickness = value; }

		auto getShockwaveForce() const { return mShockwaveForce; }
		void setShockwaveForce(float value) { mShockwaveForce = value; }

	private:
		float mShockwaveSize = 1.0f;
		float mShockwaveThickness = 1.0f;
		float mShockwaveForce = 1.0f;
	};

	template <class T> class Smoother : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");
	public:
		void updateTransform() override
		{
			auto prev_transform = T::getTransform();
			T::updateTransform();
			
			if (!mSmoothTransform)
				return;
			
			auto now = FRAME->getUptime();
			if (mPrevTransformTimepoint.has_value())
			{
				auto dTime = now - mPrevTransformTimepoint.value();
				auto new_transform = T::getTransform();
				T::setTransform(Common::Helpers::SmoothValue(prev_transform, new_transform, dTime));
			}
			mPrevTransformTimepoint = now;
		}

		void updateAbsoluteSize() override
		{
			auto prev_size = T::getAbsoluteSize();
			T::updateAbsoluteSize();

			if (!mSmoothAbsoluteSize)
				return;

			auto now = FRAME->getUptime();
			if (mPrevSizeTimepoint.has_value())
			{
				auto dTime = now - mPrevSizeTimepoint.value();
				auto new_size = T::getAbsoluteSize();
				T::setAbsoluteSize(Common::Helpers::SmoothValue(prev_size, new_size, dTime));
			}
			mPrevSizeTimepoint = now;
		}

	private:
		std::optional<Clock::Duration> mPrevTransformTimepoint;
		std::optional<Clock::Duration> mPrevSizeTimepoint;

	public:
		void setSmoothTransform(bool value) { mSmoothTransform = value; }
		void setSmoothAbsoluteSize(bool value) { mSmoothAbsoluteSize = value; }

	private:
		bool mSmoothTransform = true;
		bool mSmoothAbsoluteSize = true;
	};

	class Checkbox : public Scene::Clickable<Scene::Node>
	{
	public:
		Checkbox();

	public:
		using ChangeCallback = std::function<void(bool)>;

	public:
		auto getLabel() const { return mLabel; }
		auto getOuterRectangle() const { return mOuterRectangle; }
		auto getInnerRectangle() const { return mInnerRectangle; }

		void setChecked(bool value) { mChecked = value; }
		auto isChecked() const { return mChecked; }

		void setChangeCallback(ChangeCallback value) { mChangeCallback = value; }

	private:
		std::shared_ptr<Scene::Label> mLabel;
		std::shared_ptr<Scene::Rectangle> mOuterRectangle;
		std::shared_ptr<Scene::Rectangle> mInnerRectangle;
		bool mChecked = true;
		ChangeCallback mChangeCallback = nullptr;
	};

	class Editbox : public RectangleButton
	{
	private:
		class Window;

	public:
		Editbox();
	};

	class Editbox::Window : public StandardWindow
	{
	public:
		Window();
	};

	// 3d

	//std::vector<std::shared_ptr<Scene3D::Model>> MakeModelsFromObj(const std::string& path_to_folder, const std::string& name_without_extension);

	class CursorIndicator : public Scene::Rectangle, public std::enable_shared_from_this<CursorIndicator>
	{
	public:
		CursorIndicator(std::shared_ptr<Scene::Label> label);

		void setCursorPos(std::optional<int> value) { mCursorPos = value; }

	private:
		std::optional<int> mCursorPos;
	};

	template <class T> class StretchedToContent : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");

	public:
		void update(Clock::Duration delta) override
		{
			T::update(delta);

			mStretchToContentSize = { 0.0f, 0.0f };

			for (const auto& node : T::getNodes())
			{
				mStretchToContentSize.x = glm::max(mStretchToContentSize.x, node->getWidth());
				mStretchToContentSize.y = glm::max(mStretchToContentSize.y, node->getHeight());
			}

			if (mStretchToContentWidth)
				T::setWidth(mStretchToContentSize.x);

			if (mStretchToContentHeight)
				T::setHeight(mStretchToContentSize.y);
		}

	public:
		auto getStretchToContentSize() const { return mStretchToContentSize; }

		bool isStretchToContentWidth() const { return mStretchToContentWidth; }
		void setStretchToContentWidth(bool value) { mStretchToContentWidth = value; }

		bool isStretchToContentHeight() const { return mStretchToContentHeight; }
		void setStretchToContentHeight(bool value) { mStretchToContentHeight = value; }

	private:
		glm::vec2 mStretchToContentSize = { 0.0f, 0.0f };
		bool mStretchToContentWidth = true;
		bool mStretchToContentHeight = true;
	};
}
