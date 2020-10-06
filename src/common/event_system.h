#pragma once

#include <core/engine.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#define EVENT ENGINE->getSystem<Common::Event::System>()

namespace Common::Event
{
	class System
	{
	public:
		using ListenerHandle = void*;
		template <typename T> using ListenerCallback = std::function<void(const T&)>;

	private:
		template <typename T> struct ListenerData
		{
			ListenerCallback<T> callback;
		};

	private:
		template <typename T> auto getTypeIndex()
		{
			static auto type = mTypeCount++;
			return type;
		}

	public:
		template <typename T> ListenerHandle createListener(ListenerCallback<T> callback)
		{
			auto listener = new ListenerData<T>;
			listener->callback = callback;
			auto type_index = getTypeIndex<T>();
			mListeners[type_index].insert(listener);
			return listener;
		}

		template <typename T> void destroyListener(ListenerHandle handle)
		{
			auto listener = static_cast<ListenerData<T>*>(handle);
			auto type_index = getTypeIndex<T>();
			mListeners[type_index].erase(listener);
			delete listener;
		}

	public:
		template <typename T> void emit(const T& e)
		{
			auto type = getTypeIndex<T>();
		
			if (mListeners.count(type) == 0)
				return;

			for (auto handle : mListeners.at(type))
			{
				auto listener = static_cast<ListenerData<T>*>(handle);
				listener->callback(e);
			}
		}

	private:
		std::unordered_map<size_t, std::unordered_set<void*>> mListeners;
		size_t mTypeCount = 0;
	};

	template <typename T> class Listenable
	{
	public:
		Listenable() 
		{ 
			mHandle = EVENT->createListener<T>([this](const T& e) {
				onEvent(e);
			});
		}

		virtual ~Listenable() 
		{ 
			EVENT->destroyListener<T>(mHandle);
		}

	protected:
		virtual void onEvent(const T& e) = 0;

	private:
		System::ListenerHandle mHandle;
	};

	template <typename T> class Listener final : public Listenable<T>
	{
	public:
		using Callback = System::ListenerCallback<T>;

	public:
		Listener(Callback callback = nullptr) : Listenable<T>(), mCallback(callback) { }

	private:
		void onEvent(const T& e) override
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