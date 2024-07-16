#pragma once

#include <scene/node.h>
#include <scene/scene.h>

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

			auto parent_size = T::hasParent() ? T::getParent()->getAbsoluteSize() : T::getScene()->getViewport().size;

			auto size = mAdaptSize;
			size -= mAdaptMargin;
			size += mAdaptStretch * parent_size;

			auto scale = size / T::getAbsoluteSize();
			mAdaptScale = glm::min(scale.x, scale.y);

			auto offset = T::getPivot() * T::getAbsoluteSize();

			auto transform = T::getTransform();
			transform = glm::translate(transform, { offset, 0.0f });
			transform = glm::scale(transform, { mAdaptScale, mAdaptScale, 1.0f });
			transform = glm::translate(transform, { -offset, 0.0f });
			T::setTransform(transform);
		}

		void updateAbsoluteScale() override
		{
			T::updateAbsoluteScale();
			auto absolute_scale = T::getAbsoluteScale();
			absolute_scale *= mAdaptScale;
			T::setAbsoluteScale(absolute_scale);
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
		void setAdaptSize(float value) { setAdaptSize({ value, value }); }

		auto getAdaptMargin() const { return mAdaptMargin; }
		void setAdaptMargin(const glm::vec2& value) { mAdaptMargin = value; }
		void setAdaptMargin(float value) { setAdaptMargin({ value, value }); }

		auto getAdaptStretch() const { return mAdaptStretch; }
		void setAdaptStretch(const glm::vec2& value) { mAdaptStretch = value; }
		void setAdaptStretch(float value) { setAdaptStretch({ value, value }); }

		bool isAdaptingEnabled() const { return mAdaptingEnabled; }
		void setAdaptingEnabled(bool value) { mAdaptingEnabled = value; }

	private:
		bool mAdaptingEnabled = true;
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
		glm::vec2 mAdaptMargin = { 0.0f, 0.0f };
		glm::vec2 mAdaptStretch = { 0.0f, 0.0f };
		float mAdaptScale = 1.0f;
	};
}