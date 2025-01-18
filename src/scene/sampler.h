#pragma once

#include <skygfx/skygfx.h>

namespace Scene
{
	class Sampler
	{
	public:
		inline static skygfx::Sampler DefaultSampler = skygfx::Sampler::Nearest;

	public:
		auto getSampler() const { return mSampler; }
		void setSampler(skygfx::Sampler value) { mSampler = value; }

	private:
		skygfx::Sampler mSampler = DefaultSampler;
	};
}