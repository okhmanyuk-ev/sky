#pragma once

#include <scene/sprite.h>

namespace Scene
{
	class Blur : public Sprite
	{
	public:
		Blur();

	protected:
		void draw() override;

	private:
		std::shared_ptr<Graphics::Image> mImage = nullptr;
		glm::vec2 mPrevSize = { 0.0f, 0.0f };

	public:
		auto getBlurIntensity() const { return mBlurIntensity; }
		void setBlurIntensity(float value) { mBlurIntensity = value; }

	private:
		float mBlurIntensity = 1.0f;
	};
}
