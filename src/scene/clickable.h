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
			T::setTouchable(true);
		}
	
	protected:
		void touch(Node::Touch type, const glm::vec2& pos) override
		{
			T::touch(type, pos);

			if (!mClickEnabled && !mChoosed)
				return;

			if (type == Node::Touch::Begin)
			{
				mStartPos = pos;
				mChoosed = true;
				onChooseBegin();
			}
			else if (type == Node::Touch::Continue)
			{
				if (glm::distance(pos, mStartPos) > mChooseTolerance * PLATFORM->getScale())
				{
					mChoosed = false;
					onChooseEnd();
				}
			}
			else if (mChoosed)
			{
				mChoosed = false;
				onClick();
				onChooseEnd();
			}
		}

	protected:
		virtual void onClick()
		{
			if (mClickCallback)
				mClickCallback();
		}

		virtual void onChooseBegin()
		{
			if (mChooseBeginCallback)
				mChooseBeginCallback();
		}

		virtual void onChooseEnd()
		{
			if (mChooseEndCallback)
				mChooseEndCallback();
		}

	public:
		void setChooseBeginCallback(Callback value) { mChooseBeginCallback = value; }
		void setChooseEndCallback(Callback value) { mChooseEndCallback = value; }
		void setClickCallback(Callback value) { mClickCallback = value; }
	
		float getChooseTolerance() const { return mChooseTolerance; }
		void setChooseTolerance(float value) { mChooseTolerance = value; }

		bool isClickEnabled() const { return mClickEnabled; }
		void setClickEnabled(bool value) { mClickEnabled = value; }

		bool isChoosed() const { return mChoosed; }

	private:
		Callback mChooseBeginCallback = nullptr;
		Callback mChooseEndCallback = nullptr;
		Callback mClickCallback = nullptr;
		float mChooseTolerance = 32.0f;
		bool mClickEnabled = true;

	private:
		glm::vec2 mStartPos = { 0.0f, 0.0f };
		bool mChoosed = false;
	};
}