#pragma once

#include <scene/glass.h>

namespace Scene
{
	class Blur : public Glass
	{
	public:
		Blur();

	protected:
		void draw() override;

	public:
		auto getBlurIntensity() const { return mBlurIntensity; }
		void setBlurIntensity(float value) { mBlurIntensity = value; }

	private:
		float mBlurIntensity = 1.0f;
	};
}
