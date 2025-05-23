#pragma once
#include <scene/all.h>
#include "scene_manager.h"
#include <common/helpers.h>
#include <tinyxml2.h>
#include <sky/utils.h>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakePopupLabel(std::shared_ptr<Scene::Node> holder,
		std::shared_ptr<Scene::Node> target, const std::wstring& text, float text_size, float move_duration = 0.5f);

	struct WeightedCell
	{
		WeightedCell(std::shared_ptr<Scene::Node> node, float weight = 1.0f);
		std::shared_ptr<Scene::Node> node;
		float weight;
	};

	std::shared_ptr<Scene::Node> MakeWeightedRow(const std::vector<WeightedCell>& items);
	std::shared_ptr<Scene::Node> MakeWeightedColumn(const std::vector<WeightedCell>& items);

	void RecursiveColorSet(std::shared_ptr<Scene::Node> node, const glm::vec4& color);
	void RecursiveAlphaSet(std::shared_ptr<Scene::Node> node, float alpha);

	void ParseNodeFromXml(Scene::Node& node, const tinyxml2::XMLElement& root);
	void ParseColorFromXml(Scene::Color& node, const tinyxml2::XMLElement& root);
	void ParseSpriteFromXml(Scene::Sprite& node, const tinyxml2::XMLElement& root);
	void ParseLabelFromXml(Scene::Label& node, const tinyxml2::XMLElement& root);

	extern std::unordered_map<std::string, std::function<std::shared_ptr<Scene::Node>(const tinyxml2::XMLElement& root)>> XmlCreateFuncs;

	std::tuple<std::shared_ptr<Scene::Node>, std::unordered_map<std::string, std::shared_ptr<Scene::Node>>>
		CreateNodesFromXml(const std::string& xml);

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

	template <class T>
	class Button : public Scene::Clickable<T>
	{
	public:
		using Callback = std::function<void()>;

	public:
		virtual void refresh()
		{
            this->setClickEnabled(isActive());
		};

	protected:
		void onClick() override
		{
			Scene::Clickable<T>::onClick();

			auto executeCallback = [](auto callback) { if (callback) callback(); };
			if (isActive())
			{
				executeCallback(mActiveCallback);
				PLATFORM->haptic(Platform::System::HapticType::Medium);
			}
			else
			{
				executeCallback(mInactiveCallback);
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

	private:
		bool mActive = true;
		Callback mActiveCallback = nullptr;
		Callback mInactiveCallback = nullptr;
	};

	template <class T>
	class SoundButtonBehavior : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");
	protected:
		void onClick() override
		{
			T::onClick();
			sky::PlaySound(mClickSound);
		}

	public:
		void setClickSound(std::shared_ptr<sky::Audio::Sound> value) { mClickSound = value; }

	private:
		std::shared_ptr<sky::Audio::Sound> mClickSound = nullptr;
	};

	template <class T>
	class BouncingButtonBehavior : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");
	public:
		void updateTransform() override
		{
			T::updateTransform();

			auto offset = 0.5f * this->getAbsoluteSize();

			auto transform = this->getTransform();
			transform = glm::translate(transform, { offset, 0.0f });
			transform = glm::scale(transform, { mRelativeScale, mRelativeScale, 1.0f });
			transform = glm::translate(transform, { -offset, 0.0f });
            this->setTransform(transform);
		}

	public:
		void onChooseBegin() override
		{
			T::onChooseBegin();

			if (mChooseAnimationProcessing)
				return;

			mChooseAnimationStarted = true;
			this->runAction(Actions::Collection::MakeSequence(
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
			this->runAction(Actions::Collection::MakeSequence(
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
		std::shared_ptr<Scene::AutoScaled<Scene::Label>> mLabel;
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

			auto model = glm::scale(this->getTransform(), { this->getAbsoluteSize(), 1.0f });
			GRAPHICS->pushModelMatrix(model);
			GRAPHICS->drawLineRectangle(mOutlineColor->getColor());
			GRAPHICS->pop();
		}

	public:
		bool isOutlined() const { return mOutlined; }
		void setOutlined(bool value) { mOutlined = value; }

		auto getOutlineColor() const { return mOutlineColor; }

	private:
		bool mOutlined = true;
		std::shared_ptr<Scene::Color> mOutlineColor = std::make_shared<Scene::Color>();
	};

	template <typename T> class MovableByHand : public T
	{
		static_assert(std::is_base_of<Scene::Node, T>::value, "T must be derived from Node");
	public:
		MovableByHand()
		{
			this->setTouchable(true);
		}

	protected:
		void touch(Scene::Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			auto fixed_pos = pos;

			if (type != Scene::Node::Touch::Begin)
				this->setPosition(this->getPosition() + this->unproject(fixed_pos) - this->unproject(mPrevPosition));

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
			this->setClickCallback([this] {
				this->runAction(Actions::Collection::Kill(this->shared_from_this()));
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
		void update(sky::Duration dTime) override;
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
		void update(sky::Duration dTime) override;

	public:
		void setScrollbox(std::weak_ptr<Scene::Scrollbox> value) { mScrollbox = value; }

		auto isHidingEnabled() const { return mHidingEnabled; }
		void setHidingEnabled(bool value) { mHidingEnabled = value; }

	private:
		std::weak_ptr<Scene::Scrollbox> mScrollbox;
		std::shared_ptr<Scene::Rectangle> mIndicator;
		float mPrevScrollPosition = 0.0f;
		sky::TimePoint mScrollMoveTime = sky::Now();
		bool mAlphaAnimating = false;
		bool mHidden = false;
		bool mHidingEnabled = false;
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

	class CursorIndicator : public Scene::Rectangle, public std::enable_shared_from_this<CursorIndicator>
	{
	public:
		CursorIndicator(std::shared_ptr<Scene::Label> label);

		void setCursorPos(std::optional<int> value) { mCursorPos = value; }

	private:
		std::optional<int> mCursorPos;
	};

	namespace ImScene
	{
		bool IsMouseHovered(Scene::Node& node);
		void Tooltip(Scene::Node& holder, std::function<std::shared_ptr<Scene::Node>()> createContentCallback,
			std::optional<std::string> key = std::nullopt);
		void Tooltip(Scene::Node& holder, const std::wstring& text);

		struct HighlightSettings
		{
			glm::vec4 rect_color = glm::vec4{ sky::GetColor(sky::Color::Yellow), 0.25f };
			glm::vec4 outline_color = glm::vec4{ sky::GetColor(sky::Color::Yellow), 0.5f };
		};

		void HighlightUnderCursor(Scene::Node& holder, Scene::Node& node, HighlightSettings settings = HighlightSettings{},
			std::optional<std::string> key = std::nullopt);
	}
}
