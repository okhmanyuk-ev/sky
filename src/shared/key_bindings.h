#pragma once

#include <map>
#include <string>
#include <sky/dispatcher.h>
#include <platform/input.h>

namespace Shared
{
	class KeyBindings :
		public sky::Listenable<Platform::Input::Keyboard::Event>,
		public sky::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public sky::Listenable<Platform::Input::Mouse::ScrollEvent>
	{
	public:
		KeyBindings();
		~KeyBindings();

	public:
		void bind(const std::string& key, const std::string& command) { mBindings[key] = command; }
		void unbind(const std::string& key) { mBindings.erase(key); }
		void unbindAll() { mBindings.clear(); }

		const auto& getBindings() const { return mBindings; }

	private:
		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const Platform::Input::Mouse::ButtonEvent& e) override;
		void onEvent(const Platform::Input::Mouse::ScrollEvent& e) override;

		void keyDown(const std::string& key);
		void keyUp(const std::string& key);

	private:
		std::map<std::string, std::string> mBindings;
		std::map<std::string, std::string> mPendingReleases;
	};
}
