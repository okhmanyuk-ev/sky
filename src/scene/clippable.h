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
		void update() override
		{
			assert(ClipLayer == 0);

			T::update();
		}

		void enterDraw() override
		{
			T::enterDraw();

			ClipLayer += 1;

			Renderer::StencilMode stencil;
			stencil.enabled = true;
			stencil.writeMask = 255;
			stencil.readMask = 255;
			stencil.reference = ClipLayer;
			stencil.func = Renderer::ComparisonFunc::Equal;
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

			T::draw();

			GRAPHICS->pop();
		}

		void leaveDraw() override
		{
			GRAPHICS->pop();

			ClipLayer -= 1;

			T::leaveDraw();
		}
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
}
