#include "blur.h"
#include <imgui.h>

using namespace Scene;

Blur::Blur()
{
	mSprite = std::make_shared<Sprite>();
	mSprite->setStretch(1.0f);
	mSprite->setSampler(Renderer::Sampler::Linear);
	attach(mSprite);
}

void Blur::draw()
{
	Node::draw();

	glm::ivec2 pos;
	glm::ivec2 size;

	std::tie(pos, size) = getGlobalBounds();

	if (size != mPrevSize)
	{
		mImage = std::make_shared<Graphics::Image>(size.x, size.y, 4);
		mSprite->setTexture(std::make_shared<Renderer::Texture>(size.x, size.y));
		mPrevSize = size;
	}

	GRAPHICS->flush();
	RENDERER->readPixels(pos, size, mImage->getMemory());
	mImage->blur(mRadius);
	mSprite->getTexture()->writePixels(mImage->getWidth(), mImage->getHeight(), mImage->getChannels(), mImage->getMemory());
}