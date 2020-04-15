#pragma once

#include <renderer/sampler.h>

namespace Scene
{
	class Sampler
	{
	public:
		inline static Renderer::Sampler DefaultSampler = Renderer::Sampler::Nearest;

	public:
		auto getSampler() const { return mSampler; }
		void setSampler(Renderer::Sampler value) { mSampler = value; }

	private:
		Renderer::Sampler mSampler = DefaultSampler;
	};
}