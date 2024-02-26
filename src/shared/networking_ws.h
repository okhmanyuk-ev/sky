#pragma once

#include <map>
#include <common/frame_system.h>
#include <common/bitbuffer.h>
#include <nlohmann/json.hpp>

namespace Shared::NetworkingWS
{
	class NetCommands
	{
	public:
		static bool inline LogEvents = false;

	public:
		NetCommands();
		~NetCommands();
	};

	class Client;
#ifndef EMSCRIPTEN
	class Server;
#endif

	using connection_hdl =
		//websocketpp::connection_hdl
		std::weak_ptr<void>;

	class Channel
	{
		friend Client;
#ifndef EMSCRIPTEN
		friend Server;
#endif

	public:
		using ReadCallback = std::function<void(BitBuffer&)>;
		using SendCallback = std::function<void(BitBuffer&)>;
		using DisconnectCallback = std::function<void()>;

	public:
		virtual ~Channel() {}

	public:
		void read(BitBuffer& buf);
		void sendReliable(const std::string& name, BitBuffer& buf);
		void addMessageReader(const std::string& name, ReadCallback callback);

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		void setDisconnectCallback(DisconnectCallback value) { mDisconnectCallback = value; }
#ifndef EMSCRIPTEN
		auto getHdl() const { return mHdl; }

	private:
		void setHdl(connection_hdl value) { mHdl = value; }
#endif
	private:
		SendCallback mSendCallback = nullptr;
		DisconnectCallback mDisconnectCallback = nullptr;
		std::map<std::string, ReadCallback> mMessageReaders;
#ifndef EMSCRIPTEN
		connection_hdl mHdl;
#endif
	};

#ifndef EMSCRIPTEN
	class Server : public Common::FrameSystem::Frameable
	{
	public:
		Server(uint16_t port);
		~Server();

	public:
		virtual std::shared_ptr<Channel> createChannel() = 0;
		virtual void onChannelCreated(std::shared_ptr<Channel> channel);
		void onFrame() override;
		auto& getChannels() const { return mChannels; }
		std::tuple<std::string/*ip*/, uint16_t/*port*/> getV4AddressFromHdl(connection_hdl hdl);
		auto getPort() const { return mPort; }

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
		std::map<connection_hdl, std::shared_ptr<Channel>, std::owner_less<connection_hdl>> mChannels;
		uint16_t mPort;
	};
#endif

	class Client : public Common::FrameSystem::Frameable
	{
	public:
		Client(const std::string& url);
		~Client();

	private:
		void connect();

	public:
		virtual std::shared_ptr<Channel> createChannel() = 0;
		virtual void onChannelCreated(std::shared_ptr<Channel> channel);

	public:
		void onFrame() override;

	public:
		auto getChannel() const { return mChannel; }
		bool isConnected() const { return mChannel != nullptr; }

	private:
		std::shared_ptr<Channel> mChannel = nullptr;
		std::string mUrl;

		struct Impl;
		std::unique_ptr<Impl> mImpl;
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
		using UID = uint32_t;
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
