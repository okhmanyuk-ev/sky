#pragma once

#include <renderer/blend.h>

namespace Scene
{
	class Blend
	{
	public:
		auto getBlendMode() const { return mBlendMode; }
		void setBlendMode(Renderer::BlendMode value) { mBlendMode = value; }

	private:
		Renderer::BlendMode mBlendMode = Renderer::BlendStates::NonPremultiplied;
	};
}