#pragma once

#include <scene/sprite.h>

namespace Scene
{
	class Glass : public Sprite
	{
	protected:
		void draw() override;
		virtual std::shared_ptr<skygfx::Texture> postprocess(std::shared_ptr<skygfx::Texture> texture);
		virtual float getCapturePadding() const { return 0.0f; } // extra backbuffer pixels around bounds for postprocess

	private:
		std::shared_ptr<skygfx::Texture> mBackbufferTexture;

	public:
		auto getGenerateMipmaps() const { return mGenerateMipmaps; }
		void setGenerateMipmaps(bool value) { mGenerateMipmaps = value; }

	private:
		bool mGenerateMipmaps = false;
	};

	class BlurGlass : public Glass
	{
	public:
		BlurGlass();

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

	class GrayscaleGlass : public Glass
	{
	protected:
		void draw() override;

	public:
		auto getGrayscaleIntensity() const { return mGrayscaleIntensity; }
		void setGrayscaleIntensity(float value) { mGrayscaleIntensity = value; }

	private:
		float mGrayscaleIntensity = 1.0f;
	};

	class BloomGlass : public Glass
	{
	public:
		BloomGlass();

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

	class LiquidGlass : public Glass
	{
	public:
		static constexpr float MaxBlurLevels = 8.0f;

		LiquidGlass();

	protected:
		void draw() override;
		std::shared_ptr<skygfx::Texture> postprocess(std::shared_ptr<skygfx::Texture> texture) override;
		float getCapturePadding() const override;

	public:
		auto getRounding() const { return mRounding; }
		void setRounding(float value) { mRounding = value; }

		auto getBezel() const { return mBezel; }
		void setBezel(float value) { mBezel = value; }

		auto getRefraction() const { return mRefraction; }
		void setRefraction(float value) { mRefraction = value; }

		auto getLiquidBlur() const { return mLiquidBlur; }
		void setLiquidBlur(float value) { mLiquidBlur = glm::clamp(value, 0.0f, MaxBlurLevels); }

		auto getChromatic() const { return mChromatic; }
		void setChromatic(float value) { mChromatic = value; }

		auto getHighlight() const { return mHighlight; }
		void setHighlight(float value) { mHighlight = value; }

		auto getSaturation() const { return mSaturation; }
		void setSaturation(float value) { mSaturation = value; }

		auto getTint() const { return mTint; }
		void setTint(const glm::vec4& value) { mTint = value; }

	private:
		float mRounding = 0.5f;
		float mBezel = 24.0f;
		float mRefraction = 1.0f;
		float mLiquidBlur = 1.0f;
		float mChromatic = 0.15f;
		float mHighlight = 0.5f;
		float mSaturation = 1.2f;
		glm::vec4 mTint = { 1.0f, 1.0f, 1.0f, 0.05f };
	};
}
