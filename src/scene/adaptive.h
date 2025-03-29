#pragma once

#include <scene/node.h>
#include <scene/scene.h>
#include <common/helpers.h>

namespace Scene
{
	enum class AdaptBehavior
	{
		Width,
		Height,
		MinimalEdge,
		MaximalEdge
	};

	template <typename T>
		requires std::derived_from<T, Node>
	class Adaptive : public T
	{
	protected:
		void update(sky::Duration dTime) override
		{
			T::update(dTime);

			if (!mAdaptEnabled)
				return;

			if (!this->hasParent())
				return;

			auto parent_size = this->getParent()->getAbsoluteSize();
			auto scale = parent_size / this->getSize();

			if (mBehavior == AdaptBehavior::Width)
				mAdaptScale = scale.x;
			else if (mBehavior == AdaptBehavior::Height)
				mAdaptScale = scale.y;
			else if (mBehavior == AdaptBehavior::MinimalEdge)
				mAdaptScale = glm::min(scale.x, scale.y);
			else if (mBehavior == AdaptBehavior::MaximalEdge)
				mAdaptScale = glm::max(scale.x, scale.y);

			mAdaptScale = sky::sanitize(mAdaptScale);
		}

	public:
		void updateAbsoluteSize() override
		{
			T::updateAbsoluteSize();

			if (!mAdaptEnabled)
				return;

			auto absolute_size = this->getAbsoluteSize();
			absolute_size -= this->getSize();
			absolute_size += this->getSize() * mAdaptScale;
			this->setAbsoluteSize(absolute_size);
		}

		auto isAdaptEnabled() const { return mAdaptEnabled; }
		void setAdaptEnabled(bool value) { mAdaptEnabled = value; }

		auto getAdaptBehavior() const { return mBehavior; }
		void setAdaptBehavior(AdaptBehavior value) { mBehavior = value; }

	private:
		bool mAdaptEnabled = true;
		float mAdaptScale = 1.0f;
		AdaptBehavior mBehavior = AdaptBehavior::MinimalEdge;
	};
}