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

			skygfx::StencilMode stencil;
			stencil.write_mask = 255;
			stencil.read_mask = 255;
			stencil.reference = ClipLayer;
			stencil.func = skygfx::ComparisonFunc::LessEqual;
			stencil.depth_fail_op = skygfx::StencilOp::Keep;
			stencil.fail_op = skygfx::StencilOp::Keep;
			stencil.pass_op = skygfx::StencilOp::Keep;

			GRAPHICS->pushStencilMode(stencil);
		}

		void draw() override
		{
			skygfx::StencilMode stencil;
			stencil.write_mask = 255;
			stencil.read_mask = 255;
			stencil.reference = ClipLayer - 1;
			stencil.func = skygfx::ComparisonFunc::Equal;
			stencil.depth_fail_op = skygfx::StencilOp::Keep;
			stencil.fail_op = skygfx::StencilOp::Keep;
			stencil.pass_op = skygfx::StencilOp::IncrementSaturation;

			GRAPHICS->pushStencilMode(stencil);

			if (mDrawOnlyStencil)
			{
				auto blend = GRAPHICS->getCurrentState().blend_mode;
				blend.color_mask = { false, false, false, false };
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

			auto scissor = skygfx::Scissor();
			scissor.position = pos;
			scissor.size = size;

			GRAPHICS->pushScissor(scissor, true);
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
			if (mRounding <= 0.0f)
			{
				T::draw();
				return;
			}

			skygfx::StencilMode stencil;
			stencil.write_mask = 255;
			stencil.read_mask = 255;
			stencil.reference = 0;
			stencil.func = skygfx::ComparisonFunc::Equal;
			stencil.depth_fail_op = skygfx::StencilOp::Keep;
			stencil.fail_op = skygfx::StencilOp::Keep;
			stencil.pass_op = skygfx::StencilOp::IncrementSaturation;

			auto state = GRAPHICS->getCurrentState();
			state.stencil_mode = stencil;
			state.blend_mode.color_mask = { false, false, false, false };

			auto absolute_size = T::getAbsoluteSize();
			state.model_matrix = glm::scale(T::getTransform(), { absolute_size, 1.0f });

			GRAPHICS->push(state);
			GRAPHICS->clear(std::nullopt, std::nullopt, 0);
			GRAPHICS->drawRoundedRectangle({ Graphics::Color::White, 1.0f }, absolute_size, mRounding, false);
			GRAPHICS->pop();

			stencil.write_mask = 255;
			stencil.read_mask = 255;
			stencil.reference = 1;
			stencil.func = skygfx::ComparisonFunc::Equal;
			stencil.depth_fail_op = skygfx::StencilOp::Keep;
			stencil.fail_op = skygfx::StencilOp::Keep;
			stencil.pass_op = skygfx::StencilOp::Keep;

			GRAPHICS->pushStencilMode(stencil);

			T::draw();

			GRAPHICS->pop();
		}

	public:
		auto getRounding() const { return mRounding; }
		void setRounding(float value) { mRounding = value; }

	private:
		float mRounding = 1.0f;
	};
}
