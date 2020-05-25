#pragma once

#include <scene/node.h>

namespace Scene
{
	template <typename T> class Clickable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	public:
		using Callback = std::function<void()>;

	public:
		Clickable()
		{
			Node::setTouchable(true);
		}
	
	protected:
		void touch(Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			if (!mClickEnabled && mChooseCancelled)
				return;

			if (type == Node::Touch::Begin)
			{
				mStartPos = pos;
				mChooseCancelled = false;
				onChoose();
			}
			else if (type == Node::Touch::Continue)
			{
				if (glm::distance(pos, mStartPos) > mChooseTolerance * PLATFORM->getScale())
				{
					mChooseCancelled = true;
					onCancelChoose();
				}
			}
			else if (!mChooseCancelled)
			{
				mChooseCancelled = true;
				onCancelChoose();
				onClick();
			}
		}

	protected:
		virtual void onClick()
		{
			if (mClickCallback)
				mClickCallback();
		}

		virtual void onChoose()
		{
			if (mChooseCallback)
				mChooseCallback();
		}

		virtual void onCancelChoose()
		{
			if (mCancelChooseCallback)
				mCancelChooseCallback();
		}

	public:
		void setChooseCallback(Callback value) { mChooseCallback = value; }
		void setCancelChooseCallback(Callback value) { mCancelChooseCallback = value; }
		void setClickCallback(Callback value) { mClickCallback = value; }
	
		float getChooseTolerance() const { return mChooseTolerance; }
		void setChooseTolerance(float value) { mChooseTolerance = value; }

		bool isClickEnabled() const { return mClickEnabled; }
		void setClickEnabled(bool value) { mClickEnabled = value; }

	private:
		Callback mChooseCallback = nullptr;
		Callback mCancelChooseCallback = nullptr;
		Callback mClickCallback = nullptr;
		float mChooseTolerance = 32.0f;
		bool mClickEnabled = true;

	private:
		glm::vec2 mStartPos = { 0.0f, 0.0f };
		bool mChooseCancelled = true;
	};
}