#pragma once

#include <unordered_map>
#include <memory>
#include <cassert>

#define ENGINE Core::Engine::Context

namespace Core
{	
	class Engine final
	{
	public:
		static inline Engine* Context = nullptr;
	
	public:
		Engine() { Context = this; }

	private:
		template <typename T> auto getTypeIndex()
		{
			static auto type = mTypeCount++;
			return type;
		}

	public:
		template <class T> void addSystem(std::shared_ptr<T> value)
		{
			auto index = getTypeIndex<T>();
			assert(mSystems.count(index) == 0);
			mSystems[index] = value;
		}

		template <class T> void removeSystem()
		{
			auto index = getTypeIndex<T>();
			assert(mSystems.count(index) > 0);
			mSystems.erase(index);
		}

		template <class T> std::shared_ptr<T> getSystem()
		{
			return std::static_pointer_cast<T>(mSystems.at(getTypeIndex<T>()));
		}

	private:
		std::unordered_map<size_t, std::shared_ptr<void>> mSystems;
		size_t mTypeCount = 0;
	};
}