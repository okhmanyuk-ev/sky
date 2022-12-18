#pragma once

#ifndef EMSCRIPTEN

#include <map>
#include <common/frame_system.h>
#include <common/bitbuffer.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

namespace Shared::NetworkingWS
{
	class Client;
	class Server;

	class Channel
	{
		friend Client;
		friend Server;

	public:
		using ReadCallback = std::function<void(BitBuffer&)>;
		using SendCallback = std::function<void(BitBuffer&)>;

	public:
		virtual ~Channel() {}

	public:
		void read(BitBuffer& buf);
		void sendReliable(const std::string& name, BitBuffer& buf);
		void addMessageReader(const std::string& name, ReadCallback callback);

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		auto getHdl() const { return mHdl; }

	private:
		void setHdl(websocketpp::connection_hdl value) { mHdl = value; }

	private:
		SendCallback mSendCallback = nullptr;
		std::map<std::string, ReadCallback> mMessageReaders;
		websocketpp::connection_hdl mHdl;
	};

	class Server : public Common::FrameSystem::Frameable
	{
	private:
		using WSServer = websocketpp::server<websocketpp::config::asio>;

	public:
		Server(uint16_t port);

	public:
		virtual std::shared_ptr<Channel> createChannel() = 0;

	public:
		void onFrame() override;

	public:
		auto& getChannels() const { return mChannels; }

	private:
		WSServer mWSServer;
		std::map<websocketpp::connection_hdl, std::shared_ptr<Channel>, std::owner_less<websocketpp::connection_hdl>> mChannels;
	};

	class Client : public Common::FrameSystem::Frameable
	{
	private:
		using WSClient = websocketpp::client<websocketpp::config::asio_client>;

	public:
		Client(const std::string& url);

	private:
		void connect(const std::string& url);

	public:
		virtual std::shared_ptr<Channel> createChannel() = 0;

	public:
		void onFrame() override;

	public:
		auto getChannel() const { return mChannel; }
		bool isConnected() const { return mChannel != nullptr; }

	private:
		WSClient mWSClient;
		std::shared_ptr<Channel> mChannel = nullptr;
	};

	class SimpleChannel : public Channel
	{
	public:
		using EventCallback = std::function<void(const nlohmann::json&)>;
		class Plugin;

	public:
		SimpleChannel();

	public:
		void sendEvent(const std::string& name, const nlohmann::json& json = {});

	private:
		void onEventMessage(BitBuffer& buf);

	public:
		void addEventCallback(const std::string& name, EventCallback callback);

	private:
		std::map<std::string, EventCallback> mEvents;

	public:
		auto isShowEventLogs() const { return mShowEventLogs; }
		void setShowEventLogs(bool value) { mShowEventLogs = value; }

	private:
		bool mShowEventLogs = false;

	public:
		void addPlugin(std::shared_ptr<Plugin> plugin);

	private:
		std::list<std::shared_ptr<Plugin>> mPlugins;
	};

	class SimpleChannel::Plugin
	{
		friend SimpleChannel;
	
	protected:
		virtual std::string getKey() = 0;
		virtual void read(const nlohmann::json& json) = 0;
		void send(const nlohmann::json& json);

	public:
		using SendCallback = std::function<void(const nlohmann::json& json)>;

	protected:
		void setSendCallback(SendCallback value) { mSendCallback = value; }

	private:
		SendCallback mSendCallback = nullptr;
	};

	class Userbase
	{
	public:
		using UID = int;
		using Profile = nlohmann::json;

	public:
		std::tuple<UID, std::shared_ptr<Profile>> auth(const std::string& platform, const std::string& uuid);
		void commit(UID uid, std::shared_ptr<Profile> profile);

	public:
		void load(const nlohmann::json& json);
		void save(nlohmann::json& json);

	public:
		const auto& getProfiles() const { return mProfiles; }

	private:
		std::unordered_map<UID, std::shared_ptr<Profile>> mProfiles;
		std::unordered_map</*uuid*/std::string, UID> mUIDS;
		int mUsersCount = 0;
	};

	namespace RegularMessaging
	{
		class Base : public SimpleChannel::Plugin
		{
		protected:
			std::string getKey() override { return "regular"; };
			void read(const nlohmann::json& json) override;
			void send();

		private:
			using ReadFieldsCallback = std::function<void(const nlohmann::json& json)>;
			using SendFieldsCallback = std::function<nlohmann::json()>;

		public:
			void setReadFieldsCallback(ReadFieldsCallback value) { mReadFieldsCallback = value; }
			void setSendFieldsCallback(SendFieldsCallback value) { mSendFieldsCallback = value; }

		private:
			ReadFieldsCallback mReadFieldsCallback = nullptr;
			SendFieldsCallback mSendFieldsCallback = nullptr;
		};

		class Client : public Base
		{
		public:
			Client();
		};

		class Server : public Base { };
	}
}

#endif