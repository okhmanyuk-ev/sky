#include "ani_sprite.h"

using namespace Scene;

AniSprite::AniSprite()
{
	mSprite = std::make_shared<Sprite>();
	mSprite->setAnchor(0.5f);
	mSprite->setPivot(0.5f);
	attach(mSprite);
}

void AniSprite::update()
{
	Node::update();

	if (getWidth() <= 0.0f)
		setWidth(mSprite->getTexRegion().size.x);

	if (getHeight() <= 0.0f)
		setHeight(mSprite->getTexRegion().size.y);

	const auto& states = mAnimation->getStates();

	if (states.count(mState) == 0)
		return;
	
	if (mPlaying)
	{
		mAccumulator += FRAME->getTimeDelta();

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
	const auto& regions = mAnimation->getAtlas().getRegions();
	const auto& region = regions.at(frame);

	mSprite->setTexRegion(region);
	mSprite->setSize(region.size);

	auto w_scale = getWidth() / mMaxRegionSize.x;
	auto h_scale = getHeight() / mMaxRegionSize.y;

	auto min_scale = glm::min(w_scale, h_scale);

	mSprite->setScale(min_scale);
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

void AniSprite::setAnimation(std::shared_ptr<Graphics::Animation> value)
{ 
	mAnimation = value;
	mSprite->setTexture(value->getAtlas().getTexture());

	const auto& regions = mAnimation->getAtlas().getRegions();

	mMaxRegionSize = { 0.0f, 0.0f };

	for (const auto& [name, region] : regions)
	{
		mMaxRegionSize.x = glm::max(mMaxRegionSize.x, region.size.x);
		mMaxRegionSize.y = glm::max(mMaxRegionSize.y, region.size.y);
	}
}
