#include "blur.h"

using namespace Scene;

void Blur::draw()
{
	Node::draw();
	
	auto [pos, size] = getGlobalBounds();

	auto w = (int)glm::floor(size.x);
	auto h = (int)glm::floor(size.y);

	if (mTargetWidth != w || mTargetHeight != h)
	{
		mTargetWidth = w;
		mTargetHeight = h;

		mImage = std::make_shared<Graphics::Image>(w, h, 4);
		mTexture = std::make_shared<Renderer::Texture>(w, h, 4);
	}

	RENDERER->readPixels(pos, size, mImage->getMemory());
	mImage->blur();
	mTexture->writePixels(mImage->getWidth(), mImage->getHeight(), mImage->getChannels(), mImage->getMemory());

	if (mTexture == nullptr)
		return;

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	GRAPHICS->drawSprite(mTexture, model);
}