#pragma once

#include <renderer/shaders/generic.h>

namespace Renderer::Shaders
{
	class Shockwave : public Generic
	{
	private:
		struct alignas(16) Settings
		{
			float size = 1.0f;
			float thickness = 1.0f;
			float force = 1.0f;
		};

	public:
		Shockwave(const skygfx::Vertex::Layout& layout);

	protected:
		void update() override;

	public:
		auto getSize() const { return mSettings.size; }
		void setSize(float value) { mSettings.size = value; }

		auto getThickness() const { return mSettings.thickness; }
		void setThickness(float value) { mSettings.thickness = value; }

		auto getForce() const { return mSettings.force; }
		void setForce(float value) { mSettings.force = value; }

	private:
		Settings mSettings;
	};
}