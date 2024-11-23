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

				visible = isIntersect(left_bounds.pos, left_bounds.size, right_bounds.pos, right_bounds.size);
			}
			else
			{
				auto viewport = this->getScene()->getViewport();

				viewport.size *= PLATFORM->getScale();
				viewport.position *= PLATFORM->getScale();

				visible = isIntersect(left_bounds.pos, left_bounds.size, viewport.position, viewport.size);
			}

			this->setVisible(visible);
		}

	private:
		bool isIntersect(const glm::vec2& left_pos, const glm::vec2& left_size, 
			const glm::vec2& right_pos, const glm::vec2& right_size)
		{
			auto left_top = left_pos.y;
			auto left_bottom = left_pos.y + left_size.y;
			auto left_left = left_pos.x;
			auto left_right = left_pos.x + left_size.x;

			auto right_top = right_pos.y;
			auto right_bottom = right_pos.y + right_size.y;
			auto right_left = right_pos.x;
			auto right_right = right_pos.x + right_size.x;

			return
				left_right > right_left &&
				left_left < right_right &&
				left_bottom > right_top &&
				left_top < right_bottom;
		}

	public:
		void setCullTarget(std::weak_ptr<Node> value) { mCullTarget = value; }

	private:
		std::weak_ptr<Node> mCullTarget;
	};
}