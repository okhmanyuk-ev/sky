#pragma once

#include <scene/node.h>

namespace Scene
{
	template <class T> class AutoScaled : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	protected:
		void leaveUpdate() override
		{
			T::leaveUpdate();

			if (!mAutoScaleEnabled)
				return;

			if (!this->hasParent())
				return;

			auto parent_size = this->getParent()->getAbsoluteSize();
			auto scale = parent_size / this->getSize();
			this->setScale(glm::min(scale.x, scale.y));
		}

	public:
		bool isAutoScaleEnabled() const { return mAutoScaleEnabled; }
		void setAutoScaleEnabled(bool value) { mAutoScaleEnabled = value; }

	private:
		bool mAutoScaleEnabled = true;
	};
}