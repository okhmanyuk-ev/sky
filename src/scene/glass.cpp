#include "glass.h"

using namespace Scene;

// glass

void Glass::draw()
{
	auto [pos, size] = getGlobalBounds();

	if (size.x <= 0 || size.y <= 0)
		return;

	auto x = (int)glm::round(pos.x);
	auto y = (int)glm::round(pos.y);
	auto w = (int)glm::round(size.x);
	auto h = (int)glm::round(size.y);

	if (size != mPrevSize)
	{
		mImage = std::make_shared<Graphics::Image>(w, h, 4);
		setTexture(std::make_shared<Renderer::Texture>(w, h, mGenerateMipmaps));
		mPrevSize = size;
	}

	GRAPHICS->flush();
	RENDERER->readPixels({ x, y }, { w, h }, mImage->getMemory());

	getTexture()->writePixels(w, h, mImage->getChannels(), mImage->getMemory());

	Sprite::draw();
}

// blurred glass

BlurredGlass::BlurredGlass()
{
	setSampler(Renderer::Sampler::LinearMipmapLinear);
	setBlendMode(Renderer::BlendStates::Opaque);
	setGenerateMipmaps(true);
}

void BlurredGlass::draw()
{
	if (mBlurIntensity <= 0.0f)
		return;

	auto static shader = std::make_shared<Renderer::Shaders::MipmapBias>(Renderer::Vertex::PositionColorTexture::Layout);
	shader->setBias(mBlurIntensity * 6.0f);
	setShader(shader);

	Glass::draw();
}

// grayscaled glass

void GrayscaledGlass::draw()
{
	if (mGrayscaleIntensity <= 0.0f)
		return;

	auto static shader = std::make_shared<Renderer::Shaders::Grayscale>(Renderer::Vertex::PositionColorTexture::Layout);
	shader->setIntensity(mGrayscaleIntensity);
	setShader(shader);
	Glass::draw();
}