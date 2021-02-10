#include "networking_ws.h"
#include <console/device.h>

using namespace Shared::NetworkingWS;

// server

Server::Server(uint16_t port)
{
	mWSServer.set_access_channels(websocketpp::log::alevel::all);
	mWSServer.clear_access_channels(websocketpp::log::alevel::frame_payload);
	mWSServer.init_asio();
	mWSServer.listen(port);
	mWSServer.start_accept();
}

void Server::frame()
{
	mWSServer.poll_one();
}

// client

Client::Client(const std::string& url)
{
	mWSClient.set_access_channels(websocketpp::log::alevel::all);
	mWSClient.clear_access_channels(websocketpp::log::alevel::frame_payload);

	mWSClient.init_asio();

	using message_ptr = websocketpp::config::asio_client::message_type::ptr;

	mWSClient.set_message_handler([this](websocketpp::connection_hdl hdl, message_ptr msg){
		//
	});

	websocketpp::lib::error_code ec;
	auto con = mWSClient.get_connection(url, ec);
	if (ec) {
		LOG("could not create connection because: " + ec.message());
		return;
	}

	mWSClient.connect(con);
}

void Client::frame()
{
	mWSClient.poll_one();
}
