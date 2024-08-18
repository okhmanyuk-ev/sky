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

			auto parent_size = this->hasParent() ? this->getParent()->getAbsoluteSize() : this->getScene()->getViewport().size;

			auto size = mAdaptSize;
			size -= mAdaptMargin;
			size += mAdaptStretch * parent_size;

			auto scale = size / this->getAbsoluteSize();
			mAdaptScale = glm::min(scale.x, scale.y);

			if (mBakingAdaption)
			{
				bakeAdaption();
				return;
			}

			auto offset = this->getPivot() * this->getAbsoluteSize();

			auto transform = this->getTransform();
			transform = glm::translate(transform, { offset, 0.0f });
			transform = glm::scale(transform, { mAdaptScale, mAdaptScale, 1.0f });
			transform = glm::translate(transform, { -offset, 0.0f });
			this->setTransform(transform);
		}

		void updateAbsoluteScale() override
		{
			T::updateAbsoluteScale();
			auto absolute_scale = this->getAbsoluteScale();
			absolute_scale *= mAdaptScale;
			this->setAbsoluteScale(absolute_scale);
		}

	public:
		void bakeAdaption()
		{
			if (mAdaptScale == 0.0f)
				return;

			if (glm::isnan(mAdaptScale))
				return;

			this->setSize(this->getSize() * mAdaptScale);
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

		auto getAdaptScale() const { return mAdaptScale; }

		auto isBakingAdaption() const { return mBakingAdaption; }
		void setBakingAdaption(bool value) { mBakingAdaption = value; }

	private:
		bool mAdaptingEnabled = true;
		glm::vec2 mAdaptSize = { 0.0f, 0.0f };
		glm::vec2 mAdaptMargin = { 0.0f, 0.0f };
		glm::vec2 mAdaptStretch = { 0.0f, 0.0f };
		float mAdaptScale = 1.0f;
		bool mBakingAdaption = false;
	};
}