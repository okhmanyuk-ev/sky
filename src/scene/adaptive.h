#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Adaptive : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	public:
		void updateTransform() override
		{
			T::updateTransform();

			if (!mAdaptingEnabled)
				return;

			assert(mAdaptSize.x > 0.0f);
			assert(mAdaptSize.y > 0.0f);

			auto scale = mAdaptSize / T::getAbsoluteSize();
			mAdaptScale = glm::min(scale.x, scale.y);

			auto offset = T::getPivot() * T::getAbsoluteSize();
			
			auto transform = T::getTransform();
			transform = glm::translate(transform, { offset, 0.0f });
			transform = glm::scale(transform, { mAdaptScale, mAdaptScale, 1.0f });
			transform = glm::translate(transform, { -offset, 0.0f });
			T::setTransform(transform);
		}

	public:
		void bakeAdaption()
		{
			T::setSize(T::getSize() * mAdaptScale);
			mAdaptScale = 1.0f;
		}

	public:
		auto getAdaptSize() const { return mAdaptSize; }
		void setAdaptSize(const glm::vec2& value) { mAdaptSize = value; }
		void setAdaptSize(float value) { mAdaptSize = { value, value }; }

		bool isAdaptingEnabled() const { return mAdaptingEnabled; }
		void setAdaptingEnabled(bool value) { mAdaptingEnabled = value; }

	private:
		bool mAdaptingEnabled = true;
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
		float mAdaptScale = 1.0f;
	};
}