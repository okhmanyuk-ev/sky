#pragma once

#include <scene/sprite.h>

namespace Scene
{
	class Glass : public Sprite
	{
	protected:
		void draw() override;
		virtual std::shared_ptr<skygfx::Texture> postprocess(std::shared_ptr<skygfx::Texture> texture);

	private:
		std::shared_ptr<skygfx::Texture> mBackbufferTexture;
		glm::vec2 mPrevSize = { 0.0f, 0.0f };
		std::optional<skygfx::PixelFormat> mBackbufferFormat;

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

		auto getBlurPasses() const { return mBlurPasses; }
		void setBlurPasses(int value) { mBlurPasses = value; }

	private:
		float mBlurIntensity = 1.0f;
		int mBlurPasses = 1;
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

	class BloomedGlass : public Glass
	{
	public:
		BloomedGlass();

	protected:
		std::shared_ptr<skygfx::Texture> postprocess(std::shared_ptr<skygfx::Texture> texture) override;

	public:
		auto getBloomIntensity() const { return mBloomIntensity; }
		void setBloomIntensity(float value) { mBloomIntensity = value; }

		auto getBrightThreshold() const { return mBrightThreshold; }
		void setBrightThreshold(float value) { mBrightThreshold = value; }

	private:
		float mBloomIntensity = 0.5f;
		float mBrightThreshold = 0.7f;
	};
}
