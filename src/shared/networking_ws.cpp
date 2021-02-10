#include "networking_ws.h"
#include <console/device.h>
#include <common/buffer_helpers.h>

using namespace Shared::NetworkingWS;

// channel

void Channel::read(Common::BitBuffer& buf)
{
	auto name = Common::BufferHelpers::ReadString(buf);
	auto params = std::map<std::string, std::string>();
	while (buf.readBit())
	{
		auto key = Common::BufferHelpers::ReadString(buf);
		auto value = Common::BufferHelpers::ReadString(buf);
		params.insert({ key, value });
	}
	//if (mShowEventLogs || mEvents.count(name) == 0)
	{
		LOG("event: \"" + name + "\"");
		for (const auto& [key, value] : params)
		{
			LOGF(" - {} : {}", key, value);
		}
	}
	if (mEvents.count(name) > 0)
	{
		mEvents.at(name)(params);
	}
}

void Channel::sendEvent(const std::string& name, const std::map<std::string, std::string>& params)
{
	auto buf = Common::BitBuffer();
	Common::BufferHelpers::WriteString(buf, name);
	for (auto& [key, value] : params)
	{
		buf.writeBit(true);
		Common::BufferHelpers::WriteString(buf, key);
		Common::BufferHelpers::WriteString(buf, value);
	}
	buf.writeBit(false);
	send(buf);
}

void Channel::send(const Common::BitBuffer& buf)
{
	mSendCallback(buf);
}

void Channel::addEventCallback(const std::string& name, EventCallback callback)
{
	assert(mEvents.count(name) == 0);
	mEvents[name] = callback;
}

// server

Server::Server(uint16_t port)
{
	mWSServer.set_access_channels(websocketpp::log::alevel::all);
	mWSServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

	mWSServer.set_open_handler([this](websocketpp::connection_hdl hdl) {
		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mWSServer.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		mChannels.insert({ hdl, channel });
	});

	mWSServer.set_close_handler([this](websocketpp::connection_hdl hdl) {
		mChannels.erase(hdl);
	});

	mWSServer.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		if (mChannels.count(hdl) == 0)
			return;

		auto& payload = msg->get_raw_payload();
		auto buf = Common::BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannels.at(hdl)->read(buf);
	});

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

	mWSClient.set_open_handler([this](websocketpp::connection_hdl hdl) {
		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mWSClient.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		mHdl = hdl;
		mChannel = channel;
	});
	mWSClient.set_close_handler([this](websocketpp::connection_hdl hdl) {
		mHdl.reset();
		mChannel = nullptr;
	});

	mWSClient.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		auto& payload = msg->get_raw_payload();
		auto buf = Common::BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannel->read(buf);
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
