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
}
