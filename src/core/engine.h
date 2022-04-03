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
			auto index = std::type_index(typeid(T));
			mSystems[index] = value;
		}

		template <class T> void removeSystem()
		{
			assert(hasSystem<T>());
			auto index = std::type_index(typeid(T));
			mSystems.erase(index);
		}

		template <class T> bool hasSystem() const
		{
			auto index = std::type_index(typeid(T));
			return mSystems.contains(index);
		}

		template <class T> std::shared_ptr<T> getSystem() const
		{
			auto index = std::type_index(typeid(T));
			return std::static_pointer_cast<T>(mSystems.at(index));
		}

	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> mSystems;
	};
}