#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <renderer/render_target.h>
#include <common/event_system.h>
#include <stack>
#include <fmt/format.h>

namespace Scene
{
	template <typename T> class RenderLayer : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
		
	protected:
		void enterDraw() override
		{
			T::enterDraw();

			// https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
			// http://www.shawnhargreaves.com/blog/premultiplied-alpha-and-image-composition.html

			auto target = GRAPHICS->getRenderTarget(fmt::format("renderlayer_{}", (void*)this));

			GRAPHICS->pushRenderTarget(target);
			GRAPHICS->pushBlendMode(Renderer::BlendMode(Renderer::Blend::SrcAlpha, Renderer::Blend::InvSrcAlpha, 
				Renderer::Blend::One, Renderer::Blend::InvSrcAlpha));
			GRAPHICS->clear();
		}

		void leaveDraw() override
		{
			GRAPHICS->pop(2);

			auto target = GRAPHICS->getRenderTarget(fmt::format("renderlayer_{}", (void*)this));

			if (mPostprocessEnabled)
				postprocess(target);

			auto color = getRenderLayerColor()->getColor() * glm::vec4({ glm::vec3(getRenderLayerColor()->getAlpha()), 1.0f });

			GRAPHICS->pushBlendMode(Renderer::BlendStates::AlphaBlend);
			GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			GRAPHICS->drawSprite(target, glm::mat4(1.0f), { }, color);
			GRAPHICS->pop(2);
			
			T::leaveDraw();
		}

		virtual void postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture) { }

	public:
		bool isPostprocessEnabled() const { return mPostprocessEnabled; }
		void setPostprocessEnabled(bool value) { mPostprocessEnabled = value; }

		auto getRenderLayerColor() const { return mRenderLayerColor; }

	private:
		bool mPostprocessEnabled = true;
		std::shared_ptr<Color> mRenderLayerColor = std::make_shared<Color>();
	};
}