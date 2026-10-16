#include "glass.h"
#include <common/helpers.h> // invlerp
#include <skygfx/utils.h>
#include <fmt/format.h>

using namespace Scene;

// glass

void Glass::draw()
{
	auto bounds = getGlobalBounds();

	if (bounds.size.x <= 0 || bounds.size.y <= 0)
		return;

	// copy pixel-aligned area that fully covers the fractional bounds
	auto x = (int)glm::floor(bounds.pos.x);
	auto y = (int)glm::floor(bounds.pos.y);
	auto w = (int)glm::ceil(bounds.size.x) + 1;
	auto h = (int)glm::ceil(bounds.size.y) + 1;

	auto format = skygfx::GetBackbufferFormat();

	if (bounds.size != mPrevSize || !mBackbufferFormat.has_value() || mBackbufferFormat.value() != format)
	{
		mBackbufferTexture = std::make_shared<skygfx::Texture>(w, h, format, mGenerateMipmaps ? skygfx::GetMipCount(w, h) : 1);
		mPrevSize = bounds.size;
		mBackbufferFormat = format;
	}

	GRAPHICS->flushBatch();
	skygfx::CopyBackbufferToTexture(*mBackbufferTexture, { w, h }, { x, y });

	if (mGenerateMipmaps)
		mBackbufferTexture->generateMips();

	setTexture(postprocess(mBackbufferTexture));
	setTexRegion(Graphics::TexRegion(glm::fract(bounds.pos), bounds.size)); // exactly the pixels under the glass
	Sprite::draw();
}

std::shared_ptr<skygfx::Texture> Glass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	return texture;
}

// blurred glass

BlurredGlass::BlurredGlass()
{
	setBlendMode(skygfx::BlendStates::Opaque);
	setGenerateMipmaps(true);
	setSampler(skygfx::Sampler::Linear);
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

// bloomed glass

BloomedGlass::BloomedGlass()
{
	setBlendMode(skygfx::BlendStates::Opaque);
	setSampler(skygfx::Sampler::Linear);
}

std::shared_ptr<skygfx::Texture> BloomedGlass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	auto result = GRAPHICS->getRenderTarget(fmt::format("bloom_glass_{}", (void*)this), texture->getWidth(), texture->getHeight());
	GRAPHICS->pushCleanState();
	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear();
	skygfx::utils::passes::Bloom(texture.get(), result.get(), mBrightThreshold, mBloomIntensity);
	GRAPHICS->pop(2);
	return result;
}