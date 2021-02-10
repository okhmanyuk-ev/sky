#pragma once

#include <set>
#include <common/frame_system.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace Shared::NetworkingWS
{
	class Server : public Common::FrameSystem::Frameable
	{
	private:
		using WSServer = websocketpp::server<websocketpp::config::asio>;

	public:
		Server(uint16_t port);

	public:
		void frame() override;

	public:
		auto getClientsCount() const { return mConnections.size(); }

	private:
		WSServer mWSServer;
		std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> mConnections;
	};

	class Client : public Common::FrameSystem::Frameable
	{
	private:
		using WSClient = websocketpp::client<websocketpp::config::asio_client>;

	public:
		Client(const std::string& url);

	public:
		void frame() override;

	private:
		WSClient mWSClient;
	};
}