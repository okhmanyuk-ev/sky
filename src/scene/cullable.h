#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Cullable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	protected:
		void updateTransform() override
		{
			T::updateTransform();

			auto left_bounds = this->getGlobalBounds();	
			bool visible = false;

			if (!mCullTarget.expired())
			{
				auto right_bounds = mCullTarget.lock()->getGlobalBounds();
				visible = left_bounds.isIntersect(right_bounds);
			}
			else
			{
				auto viewport = this->getScene()->getViewport();
				viewport.size *= PLATFORM->getScale();
				viewport.position *= PLATFORM->getScale();
				auto viewport_bounds = Node::Bounds{
					.pos = viewport.position,
					.size = viewport.size
				};
				visible = left_bounds.isIntersect(viewport_bounds);
			}

			this->setVisible(visible);
		}

	public:
		void setCullTarget(std::weak_ptr<Node> value) { mCullTarget = value; }

	private:
		std::weak_ptr<Node> mCullTarget;
	};
}