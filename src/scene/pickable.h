#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Pickable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
 		
	public:
		using Callback = std::function<void(const glm::vec2&)>;

	public:
		Pickable()
		{
			T::setTouchable(true);
		}

	protected:
		void touch(Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			if (!mPickEnabled && !mPicking)
				return;

            if (!mPicking && mPickBeginParentSpace && !T::getParent()->hitTest(T::getParent()->unproject(pos)))
				return;

			auto executeCallback = [](auto callback, auto pos) { if (callback) callback(pos); return callback != nullptr; };

			if (type == Node::Touch::Begin)
			{
				mPicking = true;
				if (!executeCallback(mPickBeginCallback, pos))
				{
					pickBegin(pos);
				}
			}
			else if (type == Node::Touch::Continue && mPicking)
			{
				if (!executeCallback(mPickContinueCallback, pos))
				{
					pickContinue(pos);
				}
			}
			else if (type == Node::Touch::End && mPicking)
			{
				mPicking = false;
				if (!executeCallback(mPickEndCallback, pos))
				{
					pickEnd(pos);
				}
			}
		}

	protected:
		virtual void pickBegin(const glm::vec2& pos) { }
		virtual void pickContinue(const glm::vec2& pos) { }
		virtual void pickEnd(const glm::vec2& pos) { }

	public:
		void setPickBeginCallback(Callback value) { mPickBeginCallback = value; }
		void setPickContinueCallback(Callback value) { mPickContinueCallback = value; }
		void setPickEndCallback(Callback value) { mPickEndCallback = value; }

		bool isPickEnabled() const { return mPickEnabled; }
		void setPickEnabled(bool value) { mPickEnabled = value; }

		bool isPickBeginParentSpace() const { return mPickBeginParentSpace; }
		void setPickBeginParentSpace(bool value) { mPickBeginParentSpace = value; }

		bool isPicking() const { return mPicking; }

	private:
		Callback mPickBeginCallback = nullptr;
		Callback mPickContinueCallback = nullptr;
		Callback mPickEndCallback = nullptr;
		bool mPickEnabled = true;
		bool mPicking = false;
		bool mPickBeginParentSpace = false;
	};
}