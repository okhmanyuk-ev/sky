#pragma once

#include <renderer/all.h>

namespace Scene3D
{
	class Driver // TODO: driver isnt a good name for it, maybe State/DrawState?
	{
	public:
		auto getShader() const { return mShader; }
		void setShader(std::shared_ptr<Renderer::Shaders::Light> value) { mShader = value; }

	private:
		std::shared_ptr<Renderer::Shaders::Light> mShader;
	};
}