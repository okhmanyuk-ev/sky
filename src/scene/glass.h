#pragma once

#include <scene/sprite.h>

namespace Scene
{
	class Glass : public Sprite
	{
	protected:
		void draw() override;

	private:
		std::shared_ptr<Graphics::Image> mImage = nullptr;
		glm::vec2 mPrevSize = { 0.0f, 0.0f };

	public:
		auto getGenerateMipmaps() const { return mGenerateMipmaps; }
		void setGenerateMipmaps(bool value) { mGenerateMipmaps = value; }

	private:
		bool mGenerateMipmaps = false;
	};	
	
	class BlurredGlass : public Glass
	{
	public:
		BlurredGlass();

	protected:
		void draw() override;

	public:
		auto getBlurIntensity() const { return mBlurIntensity; }
		void setBlurIntensity(float value) { mBlurIntensity = value; }

	private:
		float mBlurIntensity = 1.0f;
	};

	class GrayscaledGlass : public Glass
	{
	protected:
		void draw() override;

	public:
		auto getGrayscaleIntensity() const { return mGrayscaleIntensity; }
		void setGrayscaleIntensity(float value) { mGrayscaleIntensity = value; }

	private:
		float mGrayscaleIntensity = 1.0f;
	};
}
