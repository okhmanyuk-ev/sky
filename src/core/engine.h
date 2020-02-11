#pragma once

#include <map>
#include <memory>

#define ENGINE Core::Engine::Context

namespace Core
{
	class System;
	
	class Engine final
	{
	public:
		static inline Engine* Context;
	
	public:
		Engine() { Context = this; }

	private:
		template <typename T> auto getTypeIndex()
		{
			static auto type = mTypeCount++;
			return type;
		}

	private:
		std::map<size_t, std::shared_ptr<void>> mSystems;
		size_t mTypeCount = 0;

	public:
		template <class T> void addSystem(std::shared_ptr<T> value)
		{
			mSystems[getTypeIndex<T>()] = value;
		}

		template <class T> std::shared_ptr<T> getSystem()
		{
			return std::static_pointer_cast<T>(mSystems.at(getTypeIndex<T>()));
		}
	};
}