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

	public:
		template <class T> void addSystem(std::shared_ptr<T> value)
		{
			auto index = std::type_index(typeid(T));
			assert(mSystems.count(index) == 0);
			mSystems[index] = value;
		}

		template <class T> void removeSystem()
		{
			auto index = std::type_index(typeid(T));
			assert(mSystems.count(index) > 0);
			mSystems.erase(index);
		}

		template <class T> std::shared_ptr<T> getSystem()
		{
			auto index = std::type_index(typeid(T));
			return std::static_pointer_cast<T>(mSystems.at(index));
		}

	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> mSystems;
	};
}