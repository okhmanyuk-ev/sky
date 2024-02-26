#include "networking_ws.h"
#include <console/device.h>
#include <common/buffer_helpers.h>
#include <common/console_commands.h>

#ifdef EMSCRIPTEN
#include <emscripten/websocket.h>
#else
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#endif

using namespace Shared::NetworkingWS;

// socket base

NetCommands::NetCommands()
{
	CONSOLE->registerCVar("net_log_events", { "bool" },
		CVAR_GETTER_BOOL(NetCommands::LogEvents), CVAR_SETTER_BOOL(NetCommands::LogEvents));
}

NetCommands::~NetCommands()
{
	CONSOLE->removeCVar("net_log_events");
}

// channel

void Channel::read(BitBuffer& buf)
{
	auto name = Common::BufferHelpers::ReadString(buf);
	
	if (mMessageReaders.count(name) == 0)
		throw std::runtime_error(("unknown message type in channel: " + name).c_str());

	mMessageReaders.at(name)(buf);
}

void Channel::sendReliable(const std::string& name, BitBuffer& buf)
{
	auto msg = BitBuffer();
	Common::BufferHelpers::WriteString(msg, name);
	msg.write(buf.getMemory(), buf.getSize());

	if (mSendCallback == nullptr)
		throw std::runtime_error("Channel: send callback is empty");

	mSendCallback(msg);
}

void Channel::addMessageReader(const std::string& name, ReadCallback callback)
{
	assert(mMessageReaders.count(name) == 0);
	mMessageReaders.insert({ name, callback });
}

// server

#ifndef EMSCRIPTEN
struct Server::Impl
{
	websocketpp::server<websocketpp::config::asio> server;
};

Server::Server(uint16_t port) :
	mPort(port),
	mImpl(std::make_unique<Impl>())
{
	mImpl->server.set_access_channels(websocketpp::log::alevel::none);
	mImpl->server.set_error_channels(websocketpp::log::alevel::none);

	mImpl->server.set_open_handler([this](websocketpp::connection_hdl hdl) {
		auto [ip, port] = getV4AddressFromHdl(hdl);
		sky::Log("{}:{} connected", ip, port);

		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mImpl->server.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		channel->setHdl(hdl);
		mChannels.insert({ hdl, channel });
		onChannelCreated(channel);
	});

	mImpl->server.set_close_handler([this](websocketpp::connection_hdl hdl) {
		auto [ip, port] = getV4AddressFromHdl(hdl);
		sky::Log("{}:{} disconnected", ip, port);

		auto channel = mChannels.at(hdl);
		if (channel->mDisconnectCallback)
			channel->mDisconnectCallback();

		mChannels.erase(hdl);
	});

	mImpl->server.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		if (mChannels.count(hdl) == 0)
			return;

		auto& payload = msg->get_raw_payload();
		auto buf = BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannels.at(hdl)->read(buf);
	});

	mImpl->server.init_asio();
	mImpl->server.listen(port);
	mImpl->server.start_accept();
}

Server::~Server()
{
}

void Server::onChannelCreated(std::shared_ptr<Channel> channel)
{
}

void Server::onFrame()
{
	mImpl->server.poll();
}

std::tuple<std::string/*ip*/, uint16_t/*port*/> Server::getV4AddressFromHdl(websocketpp::connection_hdl hdl)
{
	auto connection = mImpl->server.get_con_from_hdl(hdl);
	auto endpoint = connection->get_raw_socket().remote_endpoint();
	auto address = endpoint.address().to_v6().to_v4();
	auto ip = address.to_string();
	auto port = endpoint.port();
	return { ip, port };
}
#endif

// client

struct Client::Impl
{
#ifdef EMSCRIPTEN
	EMSCRIPTEN_WEBSOCKET_T handle = -1;
#else
	websocketpp::client<websocketpp::config::asio_client> wsclient;
#endif
};

Client::Client(const std::string& url) :
	mUrl(url),
	mImpl(std::make_unique<Impl>())
{
#ifdef EMSCRIPTEN
	// nothing for emscripten
#else
	mImpl->wsclient.set_access_channels(websocketpp::log::alevel::none);
	mImpl->wsclient.set_error_channels(websocketpp::log::alevel::none);

	mImpl->wsclient.init_asio();

	mImpl->wsclient.set_open_handler([this](websocketpp::connection_hdl hdl) {
		sky::Log("connected");
		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mImpl->wsclient.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		channel->setHdl(hdl);
		mChannel = channel;
		onChannelCreated(mChannel);
	});
	mImpl->wsclient.set_close_handler([this, url](websocketpp::connection_hdl hdl) {
		sky::Log("disconnected");
		mChannel = nullptr;
		connect();
	});
	mImpl->wsclient.set_fail_handler([this, url](websocketpp::connection_hdl hdl) {
		sky::Log("failed");
		connect();
	});

	mImpl->wsclient.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		auto& payload = msg->get_raw_payload();
		auto buf = BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannel->read(buf);
	});
#endif
	connect();
}

Client::~Client()
{
}

void Client::connect()
{
#ifdef EMSCRIPTEN
	EmscriptenWebSocketCreateAttributes attributes;
	emscripten_websocket_init_create_attributes(&attributes);
	attributes.url = mUrl.c_str();
	mImpl->handle = emscripten_websocket_new(&attributes);

	emscripten_websocket_set_onopen_callback(mImpl->handle, this, [](int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) -> int {
		sky::Log("connected");
		auto self = static_cast<Client*>(userData);
		auto channel = self->createChannel();
		channel->setSendCallback([self](const auto& buf) {
			emscripten_websocket_send_binary(self->mImpl->handle, buf.getMemory(), buf.getSize());
		});
		self->mChannel = channel;
		self->onChannelCreated(self->mChannel);
		return eventType;
	});

	emscripten_websocket_set_onclose_callback(mImpl->handle, this, [](int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) -> int{
		sky::Log("disconnected");
		auto self = static_cast<Client*>(userData);
		self->mChannel = nullptr;
		self->connect();
		return eventType;
	});

	emscripten_websocket_set_onmessage_callback(mImpl->handle, this, [] (int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) -> int {
		auto self = static_cast<Client*>(userData);
		auto buf = BitBuffer();
		buf.write(websocketEvent->data, websocketEvent->numBytes);
		buf.toStart();
		self->mChannel->read(buf);
		return eventType;
	});

	emscripten_websocket_set_onerror_callback(mImpl->handle, this, [] (int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) -> int {
		sky::Log("failed");
		auto self = static_cast<Client*>(userData);
		self->connect();
		return eventType;
	});
#else
	websocketpp::lib::error_code ec;
	auto con = mImpl->wsclient.get_connection(mUrl, ec);
	if (ec) {
		sky::Log("could not create connection because: " + ec.message());
		return;
	}

	mImpl->wsclient.connect(con);
#endif
	sky::Log("connecting");
}

void Client::onChannelCreated(std::shared_ptr<Channel> channel)
{
}

void Client::onFrame()
{
#ifdef EMSCRIPTEN
#else
	mImpl->wsclient.poll();
#endif
}

// simplechannel

SimpleChannel::SimpleChannel()
{
	addMessageReader("event", [this](auto& buf) { onEventMessage(buf); });
}

void SimpleChannel::sendEvent(const std::string& name, const nlohmann::json& json)
{
	auto buf = BitBuffer();
	Common::BufferHelpers::WriteString(buf, name);

	if (!json.empty())
	{
		auto bson = nlohmann::json::to_bson(json);
		buf.writeBitsVar(bson.size());
		buf.write(bson.data(), bson.size());
	}
	else
	{
		buf.writeBitsVar(0);
	}

	sendReliable("event", buf);
}

void SimpleChannel::onEventMessage(BitBuffer& buf)
{
	auto name = Common::BufferHelpers::ReadString(buf);

	nlohmann::json json = {};

	auto bson_size = buf.readBitsVar();
	if (bson_size > 0)
	{
		std::vector<uint8_t> bson;
		bson.resize(bson_size);
		buf.read(bson.data(), bson_size);
		json = nlohmann::json::from_bson(bson);
	}

	if (NetCommands::LogEvents || mEvents.count(name) == 0)
	{
		sky::Log("event: \"{}\", dump: \"{}\"", name, json.dump());
	}

	if (mEvents.count(name) > 0)
	{
		mEvents.at(name)(json);
	}
}

void SimpleChannel::addEventCallback(const std::string& name, EventCallback callback)
{
	assert(mEvents.count(name) == 0);
	mEvents[name] = callback;
}

void SimpleChannel::addPlugin(std::shared_ptr<Plugin> plugin)
{
	auto key = plugin->getKey();
	plugin->setSendCallback([this, key](const nlohmann::json& json){
		sendEvent(key, json);
	});
	addEventCallback(key, [this, plugin](const nlohmann::json& json) {
		plugin->read(json);
	});
	mPlugins.push_back(plugin);
}

// plugin

void SimpleChannel::Plugin::send(const nlohmann::json& json)
{
	mSendCallback(json);
}

// userbaseserver

std::tuple<Userbase::UID, std::shared_ptr<Userbase::Profile>> Userbase::auth(const std::string& platform, const std::string& uuid)
{
	if (mUIDS.count(uuid) == 0)
	{
		mUsersCount += 1;
		mUIDS.insert({ uuid, mUsersCount });
	}

	auto uid = mUIDS.at(uuid);

	if (mProfiles.count(uid) == 0)
	{
		mProfiles.insert({ uid, std::make_shared<Profile>() });
	}

	auto profile = mProfiles.at(uid);

	return { uid, profile };
}		

void Userbase::commit(UID uid, std::shared_ptr<Profile> profile)
{
	mProfiles[uid] = profile;
}

void Userbase::load(const nlohmann::json& json)
{
	auto& userbase = json["userbase"];

	mUsersCount = userbase["users_count"];

	for (const auto& user : userbase["users"])
	{
		UID uid = user["uid"];
		std::string uuid = user["uuid"];
		auto profile_dump = user["profile"];
		
		mUIDS.insert({ uuid, uid });
		
		if (!profile_dump.is_string())
			continue;
		
		auto profile = nlohmann::json::parse((std::string)profile_dump);
		
		mProfiles.insert({ uid, std::make_shared<Profile>(profile) });
	}
}

void Userbase::save(nlohmann::json& json)
{
	auto& userbase = json["userbase"];

	userbase["users_count"] = mUsersCount;

	auto& users = userbase["users"];

	for (const auto& [uuid, uid] : mUIDS)
	{
		nlohmann::json user = {
			{ "uuid", uuid },
			{ "uid", uid }
		};
	
		if (mProfiles.count(uid) > 0)
		{
			const auto& profile = mProfiles.at(uid);
			user["profile"] = profile->dump();
		}

		users.push_back(user);
	}
}

// RegularMessaging

void RegularMessaging::Base::read(const nlohmann::json& json)
{
	mReadFieldsCallback(json);
	send();
}

void RegularMessaging::Base::send()
{
	Plugin::send(mSendFieldsCallback());
}

RegularMessaging::Client::Client()
{
	FRAME->addOne([this] {
		send();
	});
}
