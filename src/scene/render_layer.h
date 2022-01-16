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

			if (!mRenderLayerEnabled)
				return;

			// https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
			// http://www.shawnhargreaves.com/blog/premultiplied-alpha-and-image-composition.html

			auto [pos, size] = T::getGlobalBounds();
			auto width = static_cast<int>(glm::floor(size.x));
			auto height = static_cast<int>(glm::floor(size.y));
			auto target = GRAPHICS->getRenderTarget(fmt::format("renderlayer_{}", (void*)this), width, height);

			auto view = glm::mat4(1.0f);
			view = glm::translate(view, { -pos, 0.0f });
			view = glm::scale(view, { PLATFORM->getScale(), PLATFORM->getScale(), 1.0f });

			GRAPHICS->pushRenderTarget(target);
			GRAPHICS->pushViewport(target);
			GRAPHICS->pushOrthoMatrix(target);
			GRAPHICS->pushViewMatrix(view);
			GRAPHICS->pushBlendMode(Renderer::BlendMode(Renderer::Blend::SrcAlpha, Renderer::Blend::InvSrcAlpha,
				Renderer::Blend::One, Renderer::Blend::InvSrcAlpha));
			GRAPHICS->clear();
		}

		void leaveDraw() override
		{
			if (!mRenderLayerEnabled)
			{
				T::leaveDraw();
				return;
			}

			auto target = GRAPHICS->getCurrentState().renderTarget;
			
			GRAPHICS->pop(5);

			if (mPostprocessEnabled)
				target = postprocess(target);

			auto color = getRenderLayerColor()->getColor() * glm::vec4({ glm::vec3(getRenderLayerColor()->getAlpha()), 1.0f });
			auto model = glm::scale(T::getTransform(), { T::getAbsoluteSize(), 1.0f });

			GRAPHICS->pushBlendMode(Renderer::BlendStates::AlphaBlend);
			GRAPHICS->pushModelMatrix(model);
			GRAPHICS->drawSprite(target, { }, color);
			GRAPHICS->pop(2);
			
			T::leaveDraw();
		}

		virtual std::shared_ptr<Renderer::RenderTarget> postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture) 
		{
			if (mPostprocessFunc)
				render_texture = mPostprocessFunc(render_texture);

			return render_texture;
		}

	public:
		using PostprocessFunc = std::function<std::shared_ptr<Renderer::RenderTarget>(std::shared_ptr<Renderer::RenderTarget>)>;

	public:
		bool isRenderLayerEnabled() const { return mRenderLayerEnabled; }
		void setRenderLayerEnabled(bool value) { mRenderLayerEnabled = value; }

		bool isPostprocessEnabled() const { return mPostprocessEnabled; }
		void setPostprocessEnabled(bool value) { mPostprocessEnabled = value; }

		auto getRenderLayerColor() const { return mRenderLayerColor; }

		void setPostprocessFunc(PostprocessFunc value) { mPostprocessFunc = value; }

	private:
		bool mRenderLayerEnabled = true;
		bool mPostprocessEnabled = true;
		std::shared_ptr<Color> mRenderLayerColor = std::make_shared<Color>();
		PostprocessFunc mPostprocessFunc = nullptr;
	};
}