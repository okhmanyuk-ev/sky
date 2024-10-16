#include "glass.h"
#include <common/helpers.h> // invlerp

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

	auto format = skygfx::GetBackbufferFormat();

	if (size != mPrevSize || !mBackbufferFormat.has_value() || mBackbufferFormat.value() != format)
	{
		setTexture(std::make_shared<skygfx::Texture>(w, h, format, skygfx::GetMipCount(w, h)));
		mPrevSize = size;
		mBackbufferFormat = format;
	}

	GRAPHICS->flushBatch();
	skygfx::ReadPixels({ x, y }, { w, h }, *getTexture());

	if (mGenerateMipmaps)
		getTexture()->generateMips();

	auto tl = project({ 0.0f, 0.0f });
	auto tr = project({ getAbsoluteWidth(), 0.0f });
	auto bl = project({ 0.0f, getAbsoluteHeight() });
	auto br = project({ getAbsoluteWidth(), getAbsoluteHeight() });

	glm::vec2 min_pos = { static_cast<float>(x), static_cast<float>(y) };
	glm::vec2 max_pos = min_pos + glm::vec2{ static_cast<float>(w), static_cast<float>(h) };

	Sprite::DirectTexCoords coords;
	coords.top_left_uv = Common::Helpers::invLerp(min_pos, max_pos, tl);
	coords.top_right_uv = Common::Helpers::invLerp(min_pos, max_pos, tr);
	coords.bottom_left_uv = Common::Helpers::invLerp(min_pos, max_pos, bl);
	coords.bottom_right_uv = Common::Helpers::invLerp(min_pos, max_pos, br);
	setDirectTexCoords(coords);

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