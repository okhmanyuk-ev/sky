#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Renderer/render_target.h>
#include <Common/event_system.h>
#include <stack>

namespace Scene
{
	template <typename T> class RenderLayer : public T, public Color,
		public Common::EventSystem::Listenable<Platform::System::ResizeEvent>
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
		static_assert(!std::is_base_of<Color, T>::value, "T must NOT be derived from Color");

	private:
		void event(const Platform::System::ResizeEvent& e) override
		{
			mTarget.reset();
			mTarget = std::make_shared<Renderer::RenderTarget>(e.width, e.height);
		}

	protected:
		void beginRender() override
		{
			T::beginRender();

			GRAPHICS->push(mTarget);
			GRAPHICS->push({ Renderer::Blend::SrcAlpha, Renderer::Blend::InvSrcAlpha, Renderer::Blend::One, Renderer::Blend::InvSrcAlpha });
			GRAPHICS->clear();
		}

		void endRender() override
		{
			GRAPHICS->pop(2);

			T::endRender();

			auto model = glm::scale(glm::mat4(1.0f), { PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 1.0f });
			auto color = getColor() * glm::vec4({ glm::vec3(getAlpha()), 1.0f });

			GRAPHICS->push(Renderer::BlendStates::AlphaBlend);
			GRAPHICS->draw(mTarget, model, { }, color);
			GRAPHICS->pop();
		}

	private:
		std::shared_ptr<Renderer::RenderTarget> mTarget = std::make_shared<Renderer::RenderTarget>(PLATFORM->getWidth(), PLATFORM->getHeight());
	};
}