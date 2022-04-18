#pragma once

#include <unordered_map>
#include <memory>
#include <cassert>
#include <typeindex>

#define ENGINE Core::Engine::Context

namespace Core
{	
	class Engine final
	{
	public:
		static inline Engine* Context = nullptr;
	
	public:
		Engine() { Context = this; }
		~Engine() { Context = nullptr; }

	public:
		template <class T> void addSystem(std::shared_ptr<T> value)
		{
			assert(!hasSystem<T>());
			SystemRef<T>() = value;
		}

		template <class T> void removeSystem()
		{
			assert(hasSystem<T>());
			SystemRef<T>() = nullptr;
		}

		template <class T> bool hasSystem() const
		{
			return SystemRef<T>() != nullptr;
		}

		template <class T> std::shared_ptr<T> getSystem() const
		{
			assert(hasSystem<T>());
			return SystemRef<T>();
		}

	private:
		template <class T> std::shared_ptr<T>& SystemRef() const
		{
			static std::shared_ptr<T> ptr = nullptr;
			return ptr;
		}
	};
}