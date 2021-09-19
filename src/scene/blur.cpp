#include "blur.h"

using namespace Scene;

Blur::Blur()
{
	setSampler(Renderer::Sampler::LinearMipmapLinear);
	setBlendMode(Renderer::BlendStates::Opaque);
	setGenerateMipmaps(true);
}

void Blur::draw()
{
	if (mBlurIntensity <= 0.0f)
		return;

	setMipmapBias(mBlurIntensity * 6.0f);

	Glass::draw();
}