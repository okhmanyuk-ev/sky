#pragma once

#include <Scene/node.h>
#include <stack>

namespace Scene
{
	template <typename T> class Clippable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	
	public:
		bool interactTest(const glm::vec2& value) const override
		{
			return T::hitTest(value);
		}

	protected:
		void beginRender() override
		{
			T::beginRender();
			
			auto bounds = T::getGlobalBounds();

			auto scissor = Renderer::Scissor();
			scissor.position.x = bounds.x;
			scissor.position.y = bounds.y;
			scissor.size.x = bounds.z - bounds.x;
			scissor.size.y = bounds.w - bounds.y;

			GRAPHICS->push(scissor);
		}

		void endRender() override
		{
			GRAPHICS->pop();
			T::endRender();
		}
	};
}
