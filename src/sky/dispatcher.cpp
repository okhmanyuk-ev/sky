#include "dispatcher.h"

size_t sky::Dispatcher::getListenersCount() const
{
	size_t result = 0;
	for (const auto& [type, listeners] : mListeners)
	{
		result += listeners.size();
	}
	return result;
}
