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
			mAutoSize = { 0.0f, 0.0f };

			for (const auto& node : this->getNodes())
			{
				mAutoSize.x = glm::max(mAutoSize.x, node->getX() + node->getAbsoluteWidth());
				mAutoSize.y = glm::max(mAutoSize.y, node->getY() + node->getAbsoluteHeight());
			}

			if (mAutoSizeWidthEnabled)
				this->setWidth(mAutoSize.x);

			if (mAutoSizeHeightEnabled)
				this->setHeight(mAutoSize.y);

			if (mAutoSizeWidthEnabled || mAutoSizeHeightEnabled)
				this->updateAbsoluteSize();
		}

	public:
		auto getAutoSize() const { return mAutoSize; }
		auto getAutoSizeWidth() const { return mAutoSize.x; }
		auto getAutoSizeHeight() const { return mAutoSize.y; }

		bool isAutoSizeWidthEnabled() const { return mAutoSizeWidthEnabled; }
		void setAutoSizeWidthEnabled(bool value) { mAutoSizeWidthEnabled = value; }

		bool isAutoSizeHeightEnabled() const { return mAutoSizeHeightEnabled; }
		void setAutoSizeHeightEnabled(bool value) { mAutoSizeHeightEnabled = value; }

	private:
		glm::vec2 mAutoSize = { 0.0f, 0.0f };
		bool mAutoSizeWidthEnabled = true;
		bool mAutoSizeHeightEnabled = true;
	};
}