#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Adaptive : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	protected:
		void update() override
		{
			adapt();
			T::update();
		}

	private:
		void adapt()
		{
			if (mAdaptSize.x <= 0.0f)
				return;

			if (mAdaptSize.y <= 0.0f)
				return;

			auto scale = mAdaptSize / T::getAbsoluteSize();

			T::setScale(glm::min(scale.x, scale.y));
		}

	public:
		auto getAdaptSize() const { return mAdaptSize; }
		void setAdaptSize(const glm::vec2& value) { mAdaptSize = value; }
		void setAdaptSize(float value) { mAdaptSize = { value, value }; }

	private:
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
	};
}