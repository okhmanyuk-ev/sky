#pragma once

#include <renderer/sampler.h>

namespace Scene
{
	class Sampler
	{
	public:
		auto getSampler() const { return mSampler; }
		void setSampler(Renderer::Sampler value) { mSampler = value; }

	private:
		Renderer::Sampler mSampler = Renderer::Sampler::Nearest;
	};
}