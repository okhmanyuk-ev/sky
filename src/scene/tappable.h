#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Tappable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	public:
		using Callback = std::function<void()>;

	public:
		Tappable()
		{
			T::setTouchable(true);
		}
	
	protected:
		void touch(Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			if (!mTapEnabled)
				return;

			if (type != Node::Touch::Begin)
				return;

			tap();
		}

	protected:
		virtual void onTap()
		{
			if (mTapCallback)
				mTapCallback();
		}

	public:
		void tap()
		{
			onTap();
		}

	public:
		void setTapCallback(Callback value) { mTapCallback = value; }

		bool isTapEnabled() const { return mTapEnabled; }
		void setTapEnabled(bool value) { mTapEnabled = value; }

	private:
		Callback mTapCallback = nullptr;
		bool mTapEnabled = true;
	};
}