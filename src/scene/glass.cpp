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

	// pixel-aligned area that fully covers the fractional bounds plus padding,
	// its size does not depend on position, so texture is not recreated while moving
	auto padding = (int)glm::ceil(getCapturePadding());
	auto origin = glm::ivec2(glm::floor(bounds.pos)) - padding;
	auto size = glm::uvec2(glm::ceil(bounds.size)) + 1u + (uint32_t)(padding * 2);

	// only the part inside the backbuffer is copied, the rest of the texture keeps old pixels
	auto backbuffer_size = glm::ivec2(skygfx::GetBackbufferWidth(), skygfx::GetBackbufferHeight());
	auto copy_min = glm::max(origin, 0);
	auto copy_size = glm::min(origin + glm::ivec2(size), backbuffer_size) - copy_min;

	if (copy_size.x <= 0 || copy_size.y <= 0)
		return;

	auto format = skygfx::GetBackbufferFormat();

	if (mBackbufferTexture == nullptr || glm::uvec2{ mBackbufferTexture->getWidth(), mBackbufferTexture->getHeight() } != size ||
		mBackbufferTexture->getFormat() != format)
	{
		mBackbufferTexture = std::make_shared<skygfx::Texture>(size.x, size.y, format, 1);
	}

	GRAPHICS->flushBatch();
	skygfx::CopyBackbufferToTexture(*mBackbufferTexture, copy_size, copy_min, copy_min - origin);

	setTexture(postprocess(mBackbufferTexture));
	setTexRegion(Graphics::TexRegion(bounds.pos - glm::vec2(origin), bounds.size)); // exactly the pixels under the glass
	Sprite::draw();
}

std::shared_ptr<skygfx::Texture> Glass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	return texture;
}

// blur glass

BlurGlass::BlurGlass()
{
	setBlendMode(skygfx::BlendStates::Opaque);
	setSampler(skygfx::Sampler::Linear);
}

void BlurGlass::draw()
{
	if (mBlurIntensity <= 0.0f)
		return;

	Glass::draw();
}

std::shared_ptr<skygfx::Texture> BlurGlass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	auto levels = glm::clamp(mBlurIntensity, 0.0f, 1.0f) * 8.0f;

	auto result = GRAPHICS->getRenderTarget(fmt::format("blur_glass_{}", (void*)this), texture->getWidth(), texture->getHeight());
	GRAPHICS->pushCleanState();
	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear();
	skygfx::utils::passes::DualFilterBlur(texture.get(), result.get(), levels);
	GRAPHICS->pop(2);
	return result;
}

// grayscale glass

void GrayscaleGlass::draw()
{
	if (mGrayscaleIntensity <= 0.0f)
		return;

	auto static effect = sky::effects::Effect<skygfx::utils::effects::Grayscale>();
	effect.uniform.intensity = mGrayscaleIntensity;
	setEffect(&effect);

	Glass::draw();
}

// bloom glass

BloomGlass::BloomGlass()
{
	setBlendMode(skygfx::BlendStates::Opaque);
	setSampler(skygfx::Sampler::Linear);
}

std::shared_ptr<skygfx::Texture> BloomGlass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	auto result = GRAPHICS->getRenderTarget(fmt::format("bloom_glass_{}", (void*)this), texture->getWidth(), texture->getHeight());
	GRAPHICS->pushCleanState();
	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear();
	skygfx::utils::passes::Bloom(texture.get(), result.get(), mBrightThreshold, mBloomIntensity);
	GRAPHICS->pop(2);
	return result;
}

// liquid glass

LiquidGlass::LiquidGlass()
{
	setBlendMode(skygfx::BlendStates::NonPremultiplied);
	setSampler(skygfx::Sampler::Linear);
}

float LiquidGlass::getCapturePadding() const
{
	// each downsample level roughly doubles blur radius, capture enough neighbours to avoid clamped edges
	return mLiquidBlur > 0.0f ? glm::exp2(glm::ceil(mLiquidBlur) + 1.0f) : 0.0f;
}

std::shared_ptr<skygfx::Texture> LiquidGlass::postprocess(std::shared_ptr<skygfx::Texture> texture)
{
	if (mLiquidBlur <= 0.0f)
		return texture;

	auto result = GRAPHICS->getRenderTarget(fmt::format("liquid_glass_{}", (void*)this), texture->getWidth(), texture->getHeight());
	GRAPHICS->pushCleanState();
	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear(); // also applies render target, so graphics restores the previous one after pop
	skygfx::utils::passes::DualFilterBlur(texture.get(), result.get(), mLiquidBlur);
	GRAPHICS->pop(2);
	return result;
}

void LiquidGlass::draw()
{
	auto size = getAbsoluteSize();
	auto half_min_side = glm::min(size.x, size.y) / 2.0f;

	auto static effect = sky::effects::Effect<sky::effects::LiquidGlass>();
	effect.uniform.tint = mTint;
	effect.uniform.size = size;
	effect.uniform.radius = glm::clamp(mRounding, 0.0f, 1.0f) * half_min_side;
	effect.uniform.bezel = glm::min(mBezel, half_min_side);
	effect.uniform.refraction = mRefraction;
	effect.uniform.chromatic = mChromatic;
	effect.uniform.highlight = mHighlight;
	effect.uniform.saturation = mSaturation;
	setEffect(&effect);

	Glass::draw();
}