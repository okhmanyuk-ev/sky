#pragma once

#include <scene/node.h>
#include <scene/sprite.h>
#include <graphics/animation.h>

namespace Scene
{
	class AniSprite : public Node
	{
	public:
		AniSprite();

	protected:
		void update() override;
		
	public:
		void randomizeProgress();

	public:
		auto getTexture() const { return mSprite->getTexture(); }
	
		auto getColor() const { return mSprite->getColor(); }
		void setColor(const glm::vec4& value) { mSprite->setColor(value); }

		float getAlpha() const { return mSprite->getAlpha(); }
		void setAlpha(float value) { mSprite->setAlpha(value); }

		float getFrequency() const { return mFrequency; }
		void setFrequency(float value) { mFrequency = value; }

		auto getState() const { return mState; }
		void setState(const std::string& value) { mState = value; }

		auto getProgress() const { return mProgress; }
		void setProgress(size_t value) { mProgress = value; }

		auto getAnimation() const { return mAnimation; }
		void setAnimation(std::shared_ptr<Renderer::Texture> texture, std::shared_ptr<Graphics::Animation> animation);
		void setAnimation(std::shared_ptr<Graphics::Animation> value);

		bool isPlaying() const { return mPlaying; }
		void setPlaying(bool value) { mPlaying = value; }

	private:
		std::shared_ptr<Sprite> mSprite = std::make_shared<Sprite>();
		float mFrequency = 10.0f;
		std::string mState = "idle";
		Clock::Duration mAccumulator = Clock::Duration::zero();
		std::shared_ptr<Graphics::Animation> mAnimation;
		size_t mProgress = 0;
		bool mPlaying = true;
		glm::vec2 mMaxRegionSize = { 0.0f, 0.0f };
	};

	template <typename T> class MappedAniSprite : public AniSprite
	{
		static_assert(std::is_enum<T>::value, "T must be enum");

	public:
		using Animation = T;
		using StateMap = std::map<Animation, std::string>;

	public:
		MappedAniSprite(const StateMap& state_map) : mStateMap(state_map) { }

	public:
		void setStateType(T value)
		{
			mStateType = value;
			setState(mStateMap.at(value));
		}

		auto getStateType() const { return mStateType; }

	private:
		StateMap mStateMap;
		T mStateType;
	};
}
