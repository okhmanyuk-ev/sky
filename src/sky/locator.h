#pragma once

#include <memory>
#include <cassert>

namespace sky
{
	template<class T>
	class Locator {
	private:
		Locator() = delete;
		Locator(const Locator&) = delete;

	public:
		static bool HasService()
		{
			return Service != nullptr;
		}

		static T* GetService()
		{
			assert(HasService());
			return Service.get();
		}

		static void Init(std::shared_ptr<T> service)
		{
			assert(!HasService());
			Service = service;
		}

		static void Reset()
		{
			assert(HasService());
			Service.reset();
		}

	private:
		static inline std::shared_ptr<T> Service;
	};
}