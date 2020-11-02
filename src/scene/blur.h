#pragma once

#include <scene/node.h>
#include <graphics/all.h>
#include <scene/sprite.h>

namespace Scene
{
	class Blur : public Node
	{
	public:
		Blur();

	protected:
		void draw() override;

	public:
		auto getRadius() const { return mRadius; }
		void setRadius(int value) { mRadius = value; }

		auto getSprite() const { return mSprite; }

	private:
		std::shared_ptr<Graphics::Image> mImage = nullptr;
		std::shared_ptr<Sprite> mSprite = nullptr;
		glm::ivec2 mPrevSize = { 0, 0 };
		int mRadius = 16;
	};
}
