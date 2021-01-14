#include "networking_ws.h"
#include <console/device.h>

namespace Shared::NetworkingWS
{
	Client::Client()
	{
		std::string uri = "ws://localhost:9002";

		mWSClient.set_access_channels(websocketpp::log::alevel::all);
		mWSClient.clear_access_channels(websocketpp::log::alevel::frame_payload);

		mWSClient.init_asio();

		using message_ptr = websocketpp::config::asio_client::message_type::ptr;

		mWSClient.set_message_handler([this](websocketpp::connection_hdl hdl, message_ptr msg){
			//
		});

		websocketpp::lib::error_code ec;
		auto con = mWSClient.get_connection(uri, ec);
		if (ec) {
			LOG("could not create connection because: " + ec.message());
			return;
		}

		mWSClient.connect(con);
		mWSClient.run();
	}
}
