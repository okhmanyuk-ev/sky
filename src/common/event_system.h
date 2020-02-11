#pragma once

#include <Core/engine.h>
#include <functional>
#include <map>
#include <list>

#define EVENT ENGINE->getSystem<Common::EventSystem>()

namespace Common
{
	class EventSystem
	{
	public:
		template <typename T> class Listenable;
		template <typename T> class Listener;

	private:
		template <typename T> auto getTypeIndex()
		{
			static auto type = mTypeCount++;
			return type;
		}

	public:
		template <typename T> void emit(const T& e)
		{
			auto type = getTypeIndex<T>();
		
			if (mListeners.count(type) == 0)
				return;

			for (auto listener : mListeners.at(type))
			{
				static_cast<Listenable<T>*>(listener)->event(e);
			}
		}

	private:
		std::map<size_t, std::list<void*>> mListeners;
		size_t mTypeCount = 0;
	};

	template <typename T> class EventSystem::Listenable
	{
		friend EventSystem;
	public:
		Listenable() { EVENT->mListeners[EVENT->getTypeIndex<T>()].push_back(this); }
		virtual ~Listenable() { EVENT->mListeners[EVENT->getTypeIndex<T>()].remove(this); }

	protected:
		virtual void event(const T& e) = 0;
	};

	template <typename T> class EventSystem::Listener final : public EventSystem::Listenable<T>
	{
	public:
		using Callback = std::function<void(const T&)>;

	public:
		Listener(Callback callback = nullptr) : Listenable<T>(), mCallback(callback) { }

	private:
		void event(const T& e) override
		{
			if (mCallback != nullptr)
			{
				mCallback(e);
			}
		}

	public:
		void setCallback(Callback value) { mCallback = value; }
		
	private:
		Callback mCallback = nullptr;
	};
}