#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Cullable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	protected:
		void update() override
		{
			T::update();

			auto bounds = T::getGlobalBounds();			
			auto viewport = T::getScene()->getViewport();

			auto b_top = bounds.y;
			auto b_bottom = bounds.w;
			auto b_left = bounds.x;
			auto b_right = bounds.z;

			auto v_top = viewport.position.y;
			auto v_bottom = viewport.position.y + viewport.size.y;
			auto v_left = viewport.position.x;
			auto v_right = viewport.position.x + viewport.size.x;

			auto visible =
				b_right > v_left &&
				b_left < v_right &&
				b_bottom > v_top &&
				b_top < v_bottom;

			T::setVisible(visible);
		}
	};
}