#pragma once

#include <scene/node.h>
#include <common/helpers.h>

namespace Scene
{
	template <typename T>
		requires std::derived_from<T, Node>
	class Eased : public T
	{
	public:
		using T::T;

	protected:
		void updateTransform() override
		{
			auto prev_transform = this->getTransform();
			T::updateTransform();

			if (!mEasedTransformEnabled)
				return;

			auto now = SCHEDULER->getUptime();
			if (mPrevTransformTimepoint.has_value())
			{
				auto dTime = now - mPrevTransformTimepoint.value();
				auto new_transform = this->getTransform();
				this->setTransform(sky::ease_towards(prev_transform, new_transform, dTime));
			}
			mPrevTransformTimepoint = now;
		}

		void updateAbsoluteSize() override
		{
			auto prev_size = this->getAbsoluteSize();
			T::updateAbsoluteSize();

			if (!mEasedAbsoluteSizeEnabled)
				return;

			auto now = SCHEDULER->getUptime();
			if (mPrevSizeTimepoint.has_value())
			{
				auto dTime = now - mPrevSizeTimepoint.value();
				auto new_size = this->getAbsoluteSize();
				this->setAbsoluteSize(sky::ease_towards(prev_size, new_size, dTime));
			}
			mPrevSizeTimepoint = now;
		}

	private:
		std::optional<sky::Duration> mPrevTransformTimepoint;
		std::optional<sky::Duration> mPrevSizeTimepoint;

	public:
		void setEasedTransformEnabled(bool value) { mEasedTransformEnabled = value; }
		void setEasedAbsoluteSizeEnabled(bool value) { mEasedAbsoluteSizeEnabled = value; }

	private:
		bool mEasedTransformEnabled = true;
		bool mEasedAbsoluteSizeEnabled = true;
	};
}