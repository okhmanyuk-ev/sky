#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace Shared::NetworkingWS
{
	class Server
	{

	};

	class Client
	{
	private:
		using WSClient = websocketpp::client<websocketpp::config::asio_client>;

	public:
		Client();

	private:
		WSClient mWSClient;
	};
}