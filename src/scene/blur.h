#pragma once

#include <scene/node.h>
#include <graphics/all.h>

namespace Scene
{
	class Blur : public Node
	{
	protected:
		void draw() override;

	private:
		std::shared_ptr<Graphics::Image> mImage = nullptr;
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		int mTargetWidth = 0;
		int mTargetHeight = 0;
	};
}
