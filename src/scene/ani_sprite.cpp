#include "ani_sprite.h"

using namespace Scene;

AniSprite::AniSprite()
{
	//mSprite->setStretch(true);
	mSprite->setAnchor({ 0.5f, 0.5f });
	mSprite->setPivot({ 0.5f, 0.5f });
	attach(mSprite);
}

void AniSprite::update()
{
	if (getWidth() <= 0.0f)
		setWidth(mSprite->getTexRegion().size.x);

	if (getHeight() <= 0.0f)
		setHeight(mSprite->getTexRegion().size.y);

	const auto& states = mAnimation->getStates();

	if (states.count(mState) == 0)
	{
		Node::update();
		return;
	}

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
	const auto& regions = mAnimation->getAtlas().getTexRegions();
	const auto& region = regions.at(frame);

	mSprite->setTexRegion(region);
	mSprite->setSize(region.size);
	
	if (!mMaxRegionSizeFound)
	{
		for (const auto& [name, region] : regions)
		{
			mMaxRegionSize.x = glm::max(mMaxRegionSize.x, region.size.x);
			mMaxRegionSize.y = glm::max(mMaxRegionSize.y, region.size.y);
		}
		mMaxRegionSizeFound = true;
	}

	auto w_scale = getWidth() / mMaxRegionSize.x;
	auto h_scale = getHeight() / mMaxRegionSize.y;

	auto min_scale = glm::min(w_scale, h_scale);

	mSprite->setScale(min_scale);

	Node::update();
}

void AniSprite::randomizeProgress()
{
	const auto& states = mAnimation->getStates();

	if (states.count(mState) == 0)
		return;

	const auto& frames = states.at(mState);

	mProgress = glm::linearRand<size_t>(0, frames.size() - 1);
}

void AniSprite::setAnimation(std::shared_ptr<Renderer::Texture> texture, std::shared_ptr<Graphics::Animation> animation) 
{ 
	mAnimation = animation; 
	mSprite->setTexture(texture); 
	mMaxRegionSizeFound = false; 
}

void AniSprite::setAnimation(std::shared_ptr<Graphics::Animation> value)
{ 
	auto& image = value->getAtlas().getImage();
	setAnimation(std::make_shared<Renderer::Texture>(image.getWidth(), image.getHeight(), image.getChannels(), image.getMemory()), value); 
}
