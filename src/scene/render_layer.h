#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>
#include <common/event_system.h>
#include <stack>
#include <fmt/format.h>

namespace Scene
{
	template <typename T> class RenderLayer : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
		
	public:
		RenderLayer()
		{
			mRenderLayerBlend->setBlendMode(skygfx::BlendStates::AlphaBlend);
		}

	protected:
		void enterDraw() override
		{
			T::enterDraw();

			if (!mRenderLayerEnabled)
				return;

			// https://stackoverflow.com/questions/2171085/opengl-blending-with-previous-contents-of-framebuffer
			// http://www.shawnhargreaves.com/blog/premultiplied-alpha-and-image-composition.html

			if (mUseLocalTargetSize)
			{
				auto [pos, size] = T::getGlobalBounds();

				assert(!glm::isnan(size.x));
				assert(!glm::isnan(size.y));

				auto width = static_cast<int>(glm::floor(size.x));
				auto height = static_cast<int>(glm::floor(size.y));

				auto target = GRAPHICS->getRenderTarget(fmt::format("renderlayer_{}", (void*)this), width, height);

				auto view = glm::mat4(1.0f);
				view = glm::translate(view, { -pos, 0.0f });
				view = glm::scale(view, { PLATFORM->getScale(), PLATFORM->getScale(), 1.0f });

				GRAPHICS->pushRenderTarget(target);
				GRAPHICS->pushOrthoMatrix(target);
				GRAPHICS->pushViewMatrix(view);
			}
			else
			{
				auto target = GRAPHICS->getRenderTarget(fmt::format("renderlayer_{}", (void*)this));

				GRAPHICS->pushRenderTarget(target);
			}

			GRAPHICS->pushBlendMode(skygfx::BlendMode(skygfx::Blend::SrcAlpha, skygfx::Blend::InvSrcAlpha,
				skygfx::Blend::One, skygfx::Blend::InvSrcAlpha));
			GRAPHICS->clear();
		}

		void leaveDraw() override
		{
			if (!mRenderLayerEnabled)
			{
				T::leaveDraw();
				return;
			}

			auto target = GRAPHICS->getCurrentState().render_target;

			GRAPHICS->pop(mUseLocalTargetSize ? 4 : 2);

			if (mPostprocessEnabled)
				target = postprocess(target);

			auto color = getRenderLayerColor()->getColor() * glm::vec4({ glm::vec3(getRenderLayerColor()->getAlpha()), 1.0f });

			if (mUseLocalTargetSize)
			{
				auto model = glm::scale(T::getTransform(), { T::getAbsoluteSize(), 1.0f });
				GRAPHICS->pushModelMatrix(model);
			}
			else
			{
				GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
			}

			GRAPHICS->pushBlendMode(mRenderLayerBlend->getBlendMode());
			GRAPHICS->drawTexturedRectangle(nullptr, target, {}, color, color, color, color);
			GRAPHICS->pop(2);

			T::leaveDraw();
		}

		virtual std::shared_ptr<skygfx::RenderTarget> postprocess(std::shared_ptr<skygfx::RenderTarget> render_texture)
		{
			if (mPostprocessFunc)
				render_texture = mPostprocessFunc(render_texture);

			return render_texture;
		}

	public:
		using PostprocessFunc = std::function<std::shared_ptr<skygfx::RenderTarget>(std::shared_ptr<skygfx::RenderTarget>)>;

	public:
		bool isRenderLayerEnabled() const { return mRenderLayerEnabled; }
		void setRenderLayerEnabled(bool value) { mRenderLayerEnabled = value; }

		bool isPostprocessEnabled() const { return mPostprocessEnabled; }
		void setPostprocessEnabled(bool value) { mPostprocessEnabled = value; }

		auto getRenderLayerColor() const { return mRenderLayerColor; }
		auto getRenderLayerBlend() const { return mRenderLayerBlend; }

		void setPostprocessFunc(PostprocessFunc value) { mPostprocessFunc = value; }

		bool isUseLocalTargetSize() const { return mUseLocalTargetSize; }
		void setUseLocalTargetSize(bool value) { mUseLocalTargetSize = value; }

	private:
		bool mRenderLayerEnabled = true;
		bool mPostprocessEnabled = true;
		std::shared_ptr<Color> mRenderLayerColor = std::make_shared<Color>();
		std::shared_ptr<Blend> mRenderLayerBlend = std::make_shared<Blend>();
		PostprocessFunc mPostprocessFunc = nullptr;
		bool mUseLocalTargetSize = true;
	};
}