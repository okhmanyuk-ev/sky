#pragma once

#include <scene/node.h>
#include <scene/scene.h>

namespace Scene
{
	template <class T> class AutoSized : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	protected:
		void leaveUpdate() override
		{
			T::leaveUpdate();
			mMaxSize = { 0.0f, 0.0f };

			for (const auto& node : this->getNodes())
			{
				mMaxSize.x = glm::max(mMaxSize.x, node->getX() + node->getAbsoluteWidth());
				mMaxSize.y = glm::max(mMaxSize.y, node->getY() + node->getAbsoluteHeight());
			}

			if (mAutoWidthEnabled)
				this->setWidth(mMaxSize.x);

			if (mAutoHeightEnabled)
				this->setHeight(mMaxSize.y);

			if (mAutoWidthEnabled || mAutoHeightEnabled)
				this->updateAbsoluteSize();
		}

	public:
		auto getMaxSize() const { return mMaxSize; }
		auto getMaxWidth() const { return mMaxSize.x; }
		auto getMaxHeight() const { return mMaxSize.y; }

		bool isAutoWidthEnabled() const { return mAutoWidthEnabled; }
		void setAutoWidthEnabled(bool value) { mAutoWidthEnabled = value; }

		bool isAutoHeightEnabled() const { return mAutoHeightEnabled; }
		void setAutoHeightEnabled(bool value) { mAutoHeightEnabled = value; }

	private:
		glm::vec2 mMaxSize = { 0.0f, 0.0f };
		bool mAutoWidthEnabled = true;
		bool mAutoHeightEnabled = true;
	};
}