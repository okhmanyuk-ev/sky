#pragma once

#include <map>
#include <common/frame_system.h>
#include <common/bitbuffer.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace Shared::NetworkingWS
{
	class Channel
	{
	public:
		using SendCallback = std::function<void(const Common::BitBuffer&)>;
		using EventCallback = std::function<void(std::map<std::string, std::string>)>;

	public:
		virtual ~Channel() {};

	public:
		void read(Common::BitBuffer& buf);

	public:
		void sendEvent(const std::string& name, const std::map<std::string, std::string>& params = {});
		void send(const Common::BitBuffer& buf);

	public:
		void addEventCallback(const std::string& name, EventCallback callback);

	private:
		std::map<std::string, EventCallback> mEvents;

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }

	private:
		SendCallback mSendCallback = nullptr;
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
		void frame() override;

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

	public:
		virtual std::shared_ptr<Channel> createChannel() = 0;

	public:
		void frame() override;

	public:
		auto getChannel() const { return mChannel; }
		bool isConnected() const { return mChannel != nullptr; }

	private:
		WSClient mWSClient;
		websocketpp::connection_hdl mHdl;
		std::shared_ptr<Channel> mChannel = nullptr;
	};
}