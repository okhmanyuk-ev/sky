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
		static bool Exists()
		{
			return Service != nullptr;
		}

		static const std::shared_ptr<T>& Get()
		{
			assert(Exists());
			return Service;
		}

		static void Set(std::shared_ptr<T> service)
		{
			assert(!Exists());
			Service = service;
		}

		template <typename... Args>
		static void Init(Args&&... args)
		{
			assert(!Exists());
			Set(std::make_shared<T>(std::forward<Args>(args)...));
		}

		static void Reset()
		{
			assert(Exists());
			Service.reset();
		}

	private:
		static inline std::shared_ptr<T> Service;
	};
}