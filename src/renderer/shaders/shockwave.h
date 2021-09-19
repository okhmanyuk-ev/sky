#pragma once

#include <renderer/shaders/default.h>

namespace Renderer::Shaders
{
	class Shockwave : public Default
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float size = 1.0f;
			float thickness = 1.0f;
			float force = 1.0f;
		};

	public:
		Shockwave(const Vertex::Layout& layout);

	public:
		auto getSize() const { return mCustomConstantBuffer.size; }
		void setSize(float value) { mCustomConstantBuffer.size = value; }

		auto getThickness() const { return mCustomConstantBuffer.thickness; }
		void setThickness(float value) { mCustomConstantBuffer.thickness = value; }

		auto getForce() const { return mCustomConstantBuffer.force; }
		void setForce(float value) { mCustomConstantBuffer.force = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}