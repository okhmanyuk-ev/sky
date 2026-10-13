#pragma once

#include <scene/node.h>
#include <scene/scene.h>
#include <common/helpers.h>
#include <sky/utils.h>

namespace Scene
{
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

			mAdaptScale = sky::CalculateScaleToFit(mBehavior, this->getSize(),
				this->getParent()->getAbsoluteSize());
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
		void setAdaptBehavior(sky::FitMode value) { mBehavior = value; }

	private:
		bool mAdaptEnabled = true;
		float mAdaptScale = 1.0f;
		sky::FitMode mBehavior = sky::FitMode::MinimalEdge;
	};
}