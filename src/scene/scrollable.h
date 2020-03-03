#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Scrollable : public Pickable<T>
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");


	protected:
		void pickBegin(const glm::vec2& pos) override
		{
			mPrevPosition = pos;
		}

		void pickContinue(const glm::vec2& pos) override
		{
			mNextPosition = pos;
			mMoved = true;
		}

		void update() override
		{
			if (mMoved)
			{
                T::setPosition(T::getPosition() + ((mNextPosition - mPrevPosition) * mSensitivity / PLATFORM->getScale()));
				mPrevPosition = mNextPosition;
				mMoved = false;
			}

			T::update();

			strictBounds();
		}

		void strictBounds()
		{
            auto myBounds = T::getGlobalBounds();
			auto parentBounds = T::getParent()->getGlobalBounds();

			bool needUpdateTransform = false;

			if (myBounds.z - myBounds.x > parentBounds.z - parentBounds.x)
			{
				if (myBounds.x > parentBounds.x)
				{
					T::setHorizontalPosition(T::getHorizontalPosition() + (parentBounds.x - myBounds.x) / PLATFORM->getScale());
					needUpdateTransform = true;
				}
				else if (myBounds.z < parentBounds.z)
				{
					T::setHorizontalPosition(T::getHorizontalPosition() + (parentBounds.z - myBounds.z) / PLATFORM->getScale());
					needUpdateTransform = true;
				}
			}

			if (myBounds.w - myBounds.y > parentBounds.w - parentBounds.y)
			{
				if (myBounds.y > parentBounds.y)
				{
					T::setVerticalPosition(T::getVerticalPosition() + (parentBounds.y - myBounds.y) / PLATFORM->getScale());
					needUpdateTransform = true;
				}
				else if (myBounds.w < parentBounds.w)
				{
					T::setVerticalPosition(T::getVerticalPosition() + (parentBounds.w - myBounds.w) / PLATFORM->getScale());
					needUpdateTransform = true;
				}
			}

			if (needUpdateTransform)
			{
				T::updateTransform();
			}
		}

	public:
		auto getSensitivity() const { return mSensitivity; }
		void setSensitivity(const glm::vec2& value) { mSensitivity = value; }

	private:
		glm::vec2 mSensitivity = { 1.0f, 1.0f };
		glm::vec2 mPrevPosition = { 0.0f, 0.0f };
		glm::vec2 mNextPosition = { 0.0f, 0.0f };
		bool mMoved = false;
	};
}