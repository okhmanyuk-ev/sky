#pragma once

#include <scene/node.h>
#include <stack>

namespace Scene
{
	inline static int ClipLayer = 0;
	
	template <typename T> class ClippableStencil : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	public:
		bool interactTest(const glm::vec2& value) const override
		{
			return T::hitTest(value);
		}

	protected:
		void update(Clock::Duration dTime) override
		{
			assert(ClipLayer == 0);
			T::update(dTime);
		}

		void enterDraw() override
		{
			T::enterDraw();

			if (ClipLayer == 0)
			{
				GRAPHICS->clear(std::nullopt, std::nullopt, 0);
			}

			ClipLayer += 1;

			Renderer::StencilMode stencil;
			stencil.enabled = true;
			stencil.writeMask = 255;
			stencil.readMask = 255;
			stencil.reference = ClipLayer;
			stencil.func = Renderer::ComparisonFunc::LessEqual;
			stencil.depthFailOp = Renderer::StencilOp::Keep;
			stencil.failOp = Renderer::StencilOp::Keep;
			stencil.passOp = Renderer::StencilOp::Keep;

			GRAPHICS->pushStencilMode(stencil);
		}

		void draw() override
		{
			Renderer::StencilMode stencil;
			stencil.enabled = true;
			stencil.writeMask = 255;
			stencil.readMask = 255;
			stencil.reference = ClipLayer - 1;
			stencil.func = Renderer::ComparisonFunc::Equal;
			stencil.depthFailOp = Renderer::StencilOp::Keep;
			stencil.failOp = Renderer::StencilOp::Keep;
			stencil.passOp = Renderer::StencilOp::IncrementSaturation;

			GRAPHICS->pushStencilMode(stencil);

			if (mDrawOnlyStencil)
			{
				auto blend = GRAPHICS->getCurrentState().blendMode;
				blend.colorMask = { false, false, false, false };
				GRAPHICS->pushBlendMode(blend);
			}

			T::draw();

			if (mDrawOnlyStencil)
			{
				GRAPHICS->pop();
			}

			GRAPHICS->pop();
		}

		void leaveDraw() override
		{
			GRAPHICS->pop();

			ClipLayer -= 1;

			T::leaveDraw();
		}


	public:
		bool isDrawOnlyStencil() const { return mDrawOnlyStencil; }
		void setDrawOnlyStencil(bool value) { mDrawOnlyStencil = value; }

	private:
		bool mDrawOnlyStencil = false;
	};

	template <typename T> class ClippableScissor : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	
	public:
		bool interactTest(const glm::vec2& value) const override
		{
			return T::hitTest(value);
		}

	protected:
		void enterDraw() override
		{
			T::enterDraw();
			
			auto [pos, size] = T::getGlobalBounds();

			auto scissor = Renderer::Scissor();
			scissor.position = pos;
			scissor.size = size;

			GRAPHICS->pushScissor(scissor);
		}

		void leaveDraw() override
		{
			GRAPHICS->pop();

			T::leaveDraw();
		}
	};

	template <class T> class Rounded : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	protected:
		void draw() override
		{
			auto state = GRAPHICS->getCurrentState();

			state.stencilMode.enabled = true;
			state.stencilMode.writeMask = 255;
			state.stencilMode.readMask = 255;
			state.stencilMode.reference = 0;
			state.stencilMode.func = Renderer::ComparisonFunc::Equal;
			state.stencilMode.depthFailOp = Renderer::StencilOp::Keep;
			state.stencilMode.failOp = Renderer::StencilOp::Keep;
			state.stencilMode.passOp = Renderer::StencilOp::IncrementSaturation;

			state.blendMode.colorMask = { false, false, false, false };

			auto absolute_size = T::getAbsoluteSize();
			state.modelMatrix = glm::scale(T::getTransform(), { absolute_size, 1.0f });

			GRAPHICS->push(state);
			GRAPHICS->clear(std::nullopt, std::nullopt, 0);
			GRAPHICS->drawRoundedRectangle({ Graphics::Color::White, 1.0f }, absolute_size, 1.0f, false);
			GRAPHICS->pop();

			Renderer::StencilMode stencil;
			stencil.enabled = true;
			stencil.writeMask = 255;
			stencil.readMask = 255;
			stencil.reference = 1;
			stencil.func = Renderer::ComparisonFunc::Equal;
			stencil.depthFailOp = Renderer::StencilOp::Keep;
			stencil.failOp = Renderer::StencilOp::Keep;
			stencil.passOp = Renderer::StencilOp::Keep;

			GRAPHICS->pushStencilMode(stencil);

			T::draw();

			GRAPHICS->pop();
		}
	};
}
