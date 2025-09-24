#include "updatable.h"
#include <sky/utils.h>

using namespace sky;

Updatable::Updatable()
{
	RunTask([](Updatable* self, std::shared_ptr<bool> finished) -> Task<> {
		while (true)
		{
			self->onFrame();
			if (!*finished)
				co_await std::suspend_always{};
		}
	}(this, mFinished));
}

Updatable::~Updatable()
{
	*mFinished = true;
}
