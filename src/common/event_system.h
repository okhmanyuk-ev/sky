#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>
#include <sky/locator.h>

#define EVENT sky::Locator<Common::Event::System>::GetService()

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

	public:
		template <typename T> ListenerHandle createListener(ListenerCallback<T> callback)
		{
			auto listener = new ListenerData<T>;
			listener->callback = callback;
			auto index = std::type_index(typeid(T));
			mListeners[index].insert(listener);
			return listener;
		}

		template <typename T> void destroyListener(ListenerHandle handle)
		{
			auto listener = static_cast<ListenerData<T>*>(handle);
			auto index = std::type_index(typeid(T));
			mListeners[index].erase(listener);
			delete listener;
		}

	public:
		template <typename T> void emit(const T& e)
		{
			auto index = std::type_index(typeid(T));

			if (mListeners.count(index) == 0)
				return;

			for (auto handle : mListeners.at(index))
			{
				auto listener = static_cast<ListenerData<T>*>(handle);
				listener->callback(e);
			}
		}

	public:
		auto getListenersCount() const
		{
			size_t result = 0;

			for (const auto& [type, listeners] : mListeners)
			{
				result += listeners.size();
			}

			return result;
		}

	private:
		std::unordered_map<std::type_index, std::unordered_set<void*>> mListeners;
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