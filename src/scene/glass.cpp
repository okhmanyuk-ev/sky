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
		setTexture(std::make_shared<Renderer::Texture>(w, h, mGenerateMipmaps));
		mPrevSize = size;
	}

	GRAPHICS->flush();
	RENDERER->readPixels({ x, y }, { w, h }, getTexture());

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
	shader->setBias(mBlurIntensity * 8.0f);
	setShader(shader);

	for (int i = 0; i < mBlurPasses; i++)
	{
		Glass::draw();
	}
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