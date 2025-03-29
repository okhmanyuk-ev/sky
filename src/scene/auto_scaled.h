#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T>
		requires std::derived_from<T, Node>
	class AutoScaled : public T
	{
	protected:
		void update(sky::Duration dTime) override
		{
			T::update(dTime);

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