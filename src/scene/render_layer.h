#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <renderer/render_target.h>
#include <common/event_system.h>
#include <stack>

namespace Scene
{
	template <typename T> class RenderLayer : public T, public Color,
		public Common::EventSystem::Listenable<Platform::System::ResizeEvent>
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
		static_assert(!std::is_base_of<Color, T>::value, "T must NOT be derived from Color");

	protected:
		void event(const Platform::System::ResizeEvent& e) override
		{
			mTarget.reset();
			mTarget = std::make_shared<Renderer::RenderTarget>(e.width, e.height);
		}

		void beginRender() override
		{
			T::beginRender();

			// https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
			// http://www.shawnhargreaves.com/blog/premultiplied-alpha-and-image-composition.html

			GRAPHICS->pushRenderTarget(mTarget);
			GRAPHICS->pushBlendMode(Renderer::BlendMode(Renderer::Blend::SrcAlpha, Renderer::Blend::InvSrcAlpha, 
				Renderer::Blend::One, Renderer::Blend::InvSrcAlpha));
			GRAPHICS->clear();
		}

		void endRender() override
		{
			GRAPHICS->pop(2);

			if (mPostprocessEnabled)
				postprocess(mTarget);

			auto color = getColor() * glm::vec4({ glm::vec3(getAlpha()), 1.0f });

			GRAPHICS->pushBlendMode(Renderer::BlendStates::AlphaBlend);
			GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			GRAPHICS->drawSprite(mTarget, glm::mat4(1.0f), { }, color);
			GRAPHICS->pop(2);
			
			T::endRender();
		}

		virtual void postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture) { }

	private:
		std::shared_ptr<Renderer::RenderTarget> mTarget = std::make_shared<Renderer::RenderTarget>(PLATFORM->getWidth(), PLATFORM->getHeight());

	public:
		bool isPostprocessEnabled() const { return mPostprocessEnabled; }
		void setPostprocessEnabled(bool value) { mPostprocessEnabled = value; }

	private:
		bool mPostprocessEnabled = true;
	};
}