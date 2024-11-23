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

	template <typename T>
	class CulledList : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	public:
		void leaveUpdate() override
		{
			T::leaveUpdate();

			auto cull_bounds = mCullTarget.lock()->getGlobalBounds();
			const auto& nodes = this->getNodes();

			for (const auto& node : nodes)
			{
				if (!node->isEnabled())
					continue;

				auto bounds = node->getGlobalBounds();
				auto visible = bounds.isIntersect(cull_bounds);
				node->setVisible(visible);
			}
			bool any_enabled = false;
			for (auto it = nodes.begin(); it != nodes.end(); ++it)
			{
				auto isAlive = [](const Node& node) {
					return node.isEnabled() && node.isVisible();
				};

				bool any_alive = false;

				if (it != nodes.begin())
					any_alive |= isAlive(**(std::prev(it)));

				if (it != std::prev(nodes.end()))
					any_alive |= isAlive(**(std::next(it)));

				any_enabled |= any_alive;
				(*it)->setEnabled(any_alive);
			}
			if (!any_enabled)
			{
				for (auto& node : nodes)
				{
					node->setEnabled(true);
				}
			}
		}

	public:
		void setCullTarget(std::weak_ptr<Node> value) { mCullTarget = value; }

	private:
		std::weak_ptr<Node> mCullTarget;
	};
}