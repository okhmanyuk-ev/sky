#pragma once

#include <memory>
#include <cassert>

namespace sky
{
	template<class T>
	class Singleton {
	private:
		Singleton() = delete;
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;

	public:
		static bool HasInstance()
		{
			return Instance != nullptr;
		}

		static T* GetInstance()
		{
			return Instance.get();
		}

		static void Init(std::shared_ptr<T> system)
		{
			assert(!HasInstance());
			Instance = system;
		}

		static void Reset()
		{
			Instance.reset();
		}

	private:
		static inline std::shared_ptr<T> Instance;
	};
}