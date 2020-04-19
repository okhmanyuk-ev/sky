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

			auto top_y = bounds.y;
			auto bottom_y = bounds.w;

			auto left_x = bounds.x;
			auto right_x = bounds.z;

			auto visible =
				left_x >= viewport.position.x &&
				right_x <= viewport.position.x + viewport.size.x &&
				bottom_y >= viewport.position.y &&
				top_y <= viewport.position.y + viewport.size.y;

			T::setVisible(visible);
		}
	};
}