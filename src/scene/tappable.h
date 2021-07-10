#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Tappable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

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

			tap(pos);
		}

	protected:
		virtual void onTap(const glm::vec2& pos)
		{
			if (mTapCallback)
				mTapCallback(pos);
		}

		virtual void onTap()
		{
			//
		}

	public:
		void tap(const glm::vec2& pos = { 0.0f, 0.0f })
		{
			onTap(pos);
			onTap();
		}

	public:
		void setTapCallback(std::function<void(const glm::vec2&)> value) { mTapCallback = value; }
		void setTapCallback(std::function<void()> value) { setTapCallback([value](auto) { value(); }); }

		bool isTapEnabled() const { return mTapEnabled; }
		void setTapEnabled(bool value) { mTapEnabled = value; }

	private:
		std::function<void(const glm::vec2&)> mTapCallback = nullptr;
		bool mTapEnabled = true;
	};
}