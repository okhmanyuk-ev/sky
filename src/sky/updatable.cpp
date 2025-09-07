#include "updatable.h"
#include <sky/scheduler.h>

using namespace sky;

Updatable::Updatable()
{
	Scheduler::Instance->add([this, finished = mFinished] {
		if (*finished)
			return Scheduler::Status::Finished;

		onFrame();
		return Scheduler::Status::Continue;
	});
}

Updatable::~Updatable()
{
	*mFinished = true;
}
