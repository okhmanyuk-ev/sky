#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>
#include <scene/sampler.h>

namespace Scene
{
	class Emitter : public Node
	{
	public:
		using CreateParticleCallback = std::function<std::shared_ptr<Node>()>;

	public:
		Emitter();

	public:
		void emit(int count = 1);

	public:
		void setHolder(std::weak_ptr<Node> value) { mHolder = value; }

		bool isRunning() const { return mRunning; }
		void setRunning(bool value) { mRunning = value; }

		auto getMinDelay() const { return mMinDelay; }
		void setMinDelay(float value) { mMinDelay = value; }

		auto getMaxDelay() const { return mMaxDelay; }
		void setMaxDelay(float value) { mMaxDelay = value; }

		void setDelay(float value) { setMinDelay(value); setMaxDelay(value); }
		void setSpawnrate(float value) { setDelay(1.0f / value); }

		auto getEndScale() const { return mEndScale; }
		void setEndScale(const glm::vec2& value) { mEndScale = value; }

		auto getDistance() const { return mDistance; }
		void setDistance(float value) { mDistance = value; }

		auto getMinDuration() const { return mMinDuration; }
		void setMinDuration(float value) { mMinDuration = value; }

		auto getMaxDuration() const { return mMaxDuration; }
		void setMaxDuration(float value) { mMaxDuration = value; }

		void setDuration(float value) { setMinDuration(value); setMaxDuration(value); }

		auto getEndColor() const { return mEndColor; }
		void setEndColor(const glm::vec4& value) { mEndColor = value; }

		auto getMinDirection() const { return mMinDirection; }
		void setMinDirection(const glm::vec2& value) { mMinDirection = value; }

		auto getMaxDirection() const { return mMaxDirection; }
		void setMaxDirection(const glm::vec2& value) { mMaxDirection = value; }

		void setDirection(const glm::vec2& value) { setMinDirection(value); setMaxDirection(value); }

		void setCreateParticleCallback(CreateParticleCallback value) { mCreateParticleCallback = value; }

	private:
		std::weak_ptr<Node> mHolder;
		bool mRunning = true;
		float mMinDelay = 0.5f;
		float mMaxDelay = 0.5f;
		glm::vec2 mEndScale = { 0.0f, 0.0f };
		float mDistance = 32.0f;
		float mMinDuration = 1.0f;
		float mMaxDuration = 1.0f;
		glm::vec4 mEndColor = { Graphics::Color::White, 0.0f };
		glm::vec2 mMinDirection = { -1.0f, -1.0f };
		glm::vec2 mMaxDirection = { 1.0f, 1.0f };
		CreateParticleCallback mCreateParticleCallback = nullptr;
	};
}
