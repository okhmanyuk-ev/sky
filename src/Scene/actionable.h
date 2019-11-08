#pragma once

#include <Common/actions.h>
#include <Scene/node.h>

namespace Scene
{
	template <typename T> class Actionable : public	T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	
	protected:
		void update() override
		{
			T::update();

			if (!mActionsEnabled)
				return;

			mActions.update();
		}

	public:
		void runAction(std::unique_ptr<Common::Actions::Action> action) { mActions.add(std::move(action)); }
		void clearActions() { mActions.clear(); }

	public:
		bool isActionsEnabled() const { return mActionsEnabled; }
		void setActionsEnabled(bool value) { mActionsEnabled = value; }

	private:
		Common::Actions::GenericActionsPlayer<Common::Actions::Parallel> mActions;
		bool mActionsEnabled = true;
	};
}