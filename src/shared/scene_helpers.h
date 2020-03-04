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

	class Emitter : public Scene::Actionable<Scene::Node>
	{
	public:
		Emitter(std::weak_ptr<Scene::Node> holder);

	public:
		void emit();

	private:
		std::weak_ptr<Scene::Node> mHolder;

	public:
		auto getMinDelay() const { return mMinDelay; }
		void setMinDelay(float value) { mMinDelay = value; }

		auto getMaxDelay() const { return mMaxDelay; }
		void setMaxDelay(float value) { mMaxDelay = value; }

		void setDelay(float value) { setMinDelay(value); setMaxDelay(value); }

		auto getParticleTexture() const { return mParticleTexture; }
		void setParticleTexture(std::shared_ptr<Renderer::Texture> value) { mParticleTexture = value; }

		auto getParticleSize() const { return mParticleSize; }
		void setParticleSize(const glm::vec2& value) { mParticleSize = value; }

		auto getDistance() const { return mDistance; }
		void setDistance(float value) { mDistance = value; }

		auto getDuration() const { return mDuration; }
		void setDuration(float value) { mDuration = value; }

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
		float mMinDelay = 0.5f;
		float mMaxDelay = 0.5f;
		std::shared_ptr<Renderer::Texture> mParticleTexture = nullptr;
		glm::vec2 mParticleSize = { 8.0f, 8.0f };
		float mDistance = 32.0f;
		float mDuration = 1.0f;
		glm::vec4 mBeginColor = { Graphics::Color::White, 1.0f };
		glm::vec4 mEndColor = { Graphics::Color::White, 0.0f };
		glm::vec2 mMinDirection = { -1.0f, -1.0f };
		glm::vec2 mMaxDirection = { 1.0f, 1.0f };
	};
}