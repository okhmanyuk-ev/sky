#include "ani_sprite.h"

using namespace Scene;

AniSprite::AniSprite()
{
	mSprite = std::make_shared<Adaptive<Sprite>>();
	mSprite->setAnchor(0.5f);
	mSprite->setPivot(0.5f);
	attach(mSprite);
}

void AniSprite::update(Clock::Duration delta)
{
	Node::update(delta);

	if (getAbsoluteWidth() <= 0.0f)
		setWidth(mSprite->getTexRegion().size.x);

	if (getAbsoluteHeight() <= 0.0f)
		setHeight(mSprite->getTexRegion().size.y);

	const auto& states = mAnimation->getStates();

	if (states.count(mState) == 0)
		return;
	
	if (mPlaying)
	{
		mAccumulator += delta;

		const auto duration = Clock::FromSeconds(1.0f / mFrequency);

		while (mAccumulator >= duration)
		{
			mAccumulator -= duration;
			mProgress += 1;
		}
	}

	const auto& frames = states.at(mState);

	while (mProgress >= frames.size())
		mProgress -= frames.size();

	const auto& frame = frames.at(mProgress);
	const auto& regions = mAtlas->getRegions();
	const auto& region = regions.at(frame);

	mSprite->setTexRegion(region);
	mSprite->setAdaptSize(mMaxRegionSize);
}

void AniSprite::randomizeProgress()
{
	if (mAnimation == nullptr)
		return;

	const auto& states = mAnimation->getStates();

	if (states.count(mState) == 0)
		return;

	const auto& frames = states.at(mState);

	mProgress = glm::linearRand<size_t>(0, frames.size() - 1);
}

void AniSprite::setAnimation(std::shared_ptr<Renderer::Texture> texture, std::shared_ptr<Graphics::Atlas> atlas, std::shared_ptr<Graphics::Animation> animation)
{ 
	mSprite->setTexture(texture);
	mAnimation = animation;;
	mAtlas = atlas;

	const auto& regions = atlas->getRegions();

	mMaxRegionSize = { 0.0f, 0.0f };

	for (const auto& [name, region] : regions)
	{
		mMaxRegionSize.x = glm::max(mMaxRegionSize.x, region.size.x);
		mMaxRegionSize.y = glm::max(mMaxRegionSize.y, region.size.y);
	}
}
