#include "networking_ws.h"
#include <console/device.h>
#include <common/buffer_helpers.h>

using namespace Shared::NetworkingWS;

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
	mSendCallback(msg);
}

void Channel::addMessageReader(const std::string& name, ReadCallback callback)
{
	assert(mMessageReaders.count(name) == 0);
	mMessageReaders.insert({ name, callback });
}

// server

Server::Server(uint16_t port)
{
	mWSServer.set_access_channels(websocketpp::log::alevel::none);
	mWSServer.set_error_channels(websocketpp::log::alevel::none);

	mWSServer.set_open_handler([this](websocketpp::connection_hdl hdl) {
		auto connection = mWSServer.get_con_from_hdl(hdl);
		LOGF("{} connected", connection->get_remote_endpoint());

		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mWSServer.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		channel->setHdl(hdl);
		mChannels.insert({ hdl, channel });
	});

	mWSServer.set_close_handler([this](websocketpp::connection_hdl hdl) {
		auto connection = mWSServer.get_con_from_hdl(hdl);
		LOGF("{} disconnected", connection->get_remote_endpoint());

		mChannels.erase(hdl);
	});

	mWSServer.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		if (mChannels.count(hdl) == 0)
			return;

		auto& payload = msg->get_raw_payload();
		auto buf = BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannels.at(hdl)->read(buf);
	});

	mWSServer.init_asio();
	mWSServer.listen(port);
	mWSServer.start_accept();
}

void Server::onFrame()
{
	mWSServer.poll();
}

// client

Client::Client(const std::string& url)
{
	mWSClient.set_access_channels(websocketpp::log::alevel::none);
	mWSClient.set_error_channels(websocketpp::log::alevel::none);

	mWSClient.init_asio();

	mWSClient.set_open_handler([this](websocketpp::connection_hdl hdl) {
		LOG("connected");
		auto channel = createChannel();
		channel->setSendCallback([this, hdl](const auto& buf) {
			mWSClient.send(hdl, buf.getMemory(), buf.getSize(), websocketpp::frame::opcode::BINARY);
		});
		channel->setHdl(hdl);
		mChannel = channel;
	});
	mWSClient.set_close_handler([this, url](websocketpp::connection_hdl hdl) {
		LOG("disconnected");
		mChannel = nullptr;
		connect(url);
	});
	mWSClient.set_fail_handler([this, url](websocketpp::connection_hdl hdl) {
		LOG("failed");
		connect(url);
	});

	mWSClient.set_message_handler([this](websocketpp::connection_hdl hdl, websocketpp::config::asio_client::message_type::ptr msg) {
		auto& payload = msg->get_raw_payload();
		auto buf = BitBuffer();
		buf.write(payload.data(), payload.size());
		buf.toStart();
		mChannel->read(buf);
	});

	connect(url);
}

void Client::connect(const std::string& url)
{
	websocketpp::lib::error_code ec;
	auto con = mWSClient.get_connection(url, ec);
	if (ec) {
		LOG("could not create connection because: " + ec.message());
		return;
	}

	mWSClient.connect(con);
	LOG("connecting");
}

void Client::onFrame()
{
	mWSClient.poll();
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

	if (mShowEventLogs || mEvents.count(name) == 0)
	{
		LOGF("event: \"{}\", dump: \"{}\"", name, json.dump());
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
