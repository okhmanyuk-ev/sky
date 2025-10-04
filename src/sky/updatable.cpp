#include "updatable.h"
#include <sky/utils.h>

using namespace sky;

Updatable::Updatable()
{
	RunAction([this, finished = mFinished] {
		if (*finished)
			return Action::Result::Finished;

		onFrame();
		return Action::Result::Continue;
	});
}

Updatable::~Updatable()
{
	*mFinished = true;
}
