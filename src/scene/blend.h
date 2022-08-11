#pragma once

#include <renderer/all.h>

namespace Scene
{
	class Blend
	{
	public:
		auto getBlendMode() const { return mBlendMode; }
		void setBlendMode(skygfx::BlendMode value) { mBlendMode = value; }

	private:
		skygfx::BlendMode mBlendMode = skygfx::BlendStates::NonPremultiplied;
	};
}