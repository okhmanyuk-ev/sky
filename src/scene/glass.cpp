#include "glass.h"
#include <common/helpers.h> // invlerp

using namespace Scene;

// glass

void Glass::draw()
{
	auto bounds = getGlobalBounds();

	if (bounds.size.x <= 0 || bounds.size.y <= 0)
		return;

	auto x = (int)glm::round(bounds.pos.x);
	auto y = (int)glm::round(bounds.pos.y);
	auto w = (int)glm::round(bounds.size.x);
	auto h = (int)glm::round(bounds.size.y);

	auto format = skygfx::GetBackbufferFormat();

	if (bounds.size != mPrevSize || !mBackbufferFormat.has_value() || mBackbufferFormat.value() != format)
	{
		setTexture(std::make_shared<skygfx::Texture>(w, h, format, skygfx::GetMipCount(w, h)));
		mPrevSize = bounds.size;
		mBackbufferFormat = format;
	}

	GRAPHICS->flushBatch();
	skygfx::ReadPixels({ x, y }, { w, h }, *getTexture());

	if (mGenerateMipmaps)
		getTexture()->generateMips();

	Sprite::draw();
}

// blurred glass

BlurredGlass::BlurredGlass()
{
	setBlendMode(skygfx::BlendStates::Opaque);
	setGenerateMipmaps(true);
}

void BlurredGlass::draw()
{
	if (mBlurIntensity <= 0.0f)
		return;

	GRAPHICS->pushMipmapBias(mBlurIntensity * 8.0f);

	if (mBlurPasses < 1)
		mBlurPasses = 1;

	for (int i = 0; i < mBlurPasses; i++)
	{
		Glass::draw();
	}

	GRAPHICS->pop();
}

// grayscaled glass

void GrayscaledGlass::draw()
{
	if (mGrayscaleIntensity <= 0.0f)
		return;

	auto static effect = sky::effects::Effect<skygfx::utils::effects::Grayscale>();
	effect.uniform.intensity = mGrayscaleIntensity;
	setEffect(&effect);

	Glass::draw();
}