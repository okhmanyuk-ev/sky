#pragma once

#include <common/actions.h>
#include <scene/node.h>

namespace Scene
{
	template <typename T> class Actionable : public	T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");
	
	protected:
		void update() override
		{
			if (mActionsEnabled)
				mActions.update();

			T::update();
		}

	public:
		void runAction(std::unique_ptr<Actions::Action> action) { mActions.add(std::move(action)); }
		void clearActions() { mActions.clear(); }
		bool hasActions() const { return mActions.hasActions(); }

	public:
		bool isActionsEnabled() const { return mActionsEnabled; }
		void setActionsEnabled(bool value) { mActionsEnabled = value; }

	private:
		Actions::GenericActionsPlayer<Actions::Parallel> mActions;
		bool mActionsEnabled = true;
	};
}