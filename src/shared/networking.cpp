#include "networking.h"
#include <console/device.h>
#include <common/helpers.h>
#include <common/buffer_helpers.h>
#include <common/actions.h>
#include <shared/stats_system.h>
#include <console/system.h>
#include <common/console_commands.h>

// #define NET_STATS(KEY, VALUE) STATS_INDICATE_GROUP("net", KEY, VALUE) // TODO: make workable in server app

using namespace Shared::Networking;

// channel

void Channel::frame()
{
	auto now = Clock::Now();

	if (now - mIncomingTime >= Clock::FromSeconds(Networking::NetTimeout))
	{
		disconnect("timed out");
		return;
	}

	if (!mReliableMessages.empty() && isReliableAcknowledged())
	{
		mOutgoingReliableSequence = !mOutgoingReliableSequence; // start reliable sending
	}

	if (!isReliableAcknowledged())
		awake(); // we want send reliables very fast while they not acknowledged

	auto durationSinceAwake = Clock::ToSeconds(now - mAwakeTime);
	mTransmitDuration = (durationSinceAwake - 0.5f) / 5.0f;
	mTransmitDuration = glm::clamp(mTransmitDuration, 0.0f, 1.0f);

	auto min_duration = Clock::ToSeconds(Clock::FromMilliseconds(Networking::NetTransmitDurationMin));
	auto max_duration = Clock::ToSeconds(Clock::FromMilliseconds(Networking::NetTransmitDurationMax));
	auto transmit_duration = Clock::FromSeconds(glm::lerp(min_duration, max_duration, mTransmitDuration));

	if (now - mTransmitTime < transmit_duration)
		return;

	mTransmitTime = now;

	transmit();
}

void Channel::transmit()
{
	mOutgoingSequence += 1;

	auto buf = Common::BitBuffer();

	bool rel_data = !isReliableAcknowledged();

	buf.writeBitsVar(mOutgoingSequence);
	buf.writeBitsVar(mIncomingSequence);
	buf.writeBit(mOutgoingReliableSequence);
	buf.writeBit(mIncomingReliableSequence);
	buf.writeBit(rel_data);

	if (rel_data)
	{
		auto& [name, msg] = mReliableMessages.front();
		buf.writeBit(true);
		Common::BufferHelpers::WriteString(buf, name);
		Common::BufferHelpers::WriteToBuffer(*msg, buf);
	}

	buf.writeBit(false);

	if (Networking::NetLogs == 3 || Networking::NetLogs == 4)
	{
		LOG("[OUT] seq: " + std::to_string(mOutgoingSequence) + 
			", ack: " + std::to_string(mIncomingSequence) + 
			", rel_seq: " + std::to_string(mOutgoingReliableSequence) +
			", rel_ack: " + std::to_string(mIncomingReliableSequence) +
			", rel_data: " + std::to_string(rel_data) + 
			", size: " + Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	mSendCallback(buf);
}

void Channel::awake()
{
	mAwakeTime = Clock::Now();
}

bool Channel::isReliableAcknowledged() const
{
	return mOutgoingReliableSequence == mIncomingReliableAcknowledgement;
}

void Channel::readReliableDataFromPacket(Common::BitBuffer& buf)
{
	while (buf.readBit())
	{
		auto msg = Common::BufferHelpers::ReadString(buf);

		if (mMessageReaders.count(msg) == 0)
			throw std::runtime_error(("unknown message type in channel: " + msg).c_str());

		mMessageReaders.at(msg)(buf);
	}
}

void Channel::read(Common::BitBuffer& buf)
{
	auto seq = buf.readBitsVar();
	auto ack = buf.readBitsVar();
	auto rel_seq = buf.readBit();
	auto rel_ack = buf.readBit();
	auto rel_data = buf.readBit();

	if (Networking::NetLogs == 2 || Networking::NetLogs == 4)
	{
		LOG("[IN ] seq: " + std::to_string(seq) + 
			", ack: " + std::to_string(ack) + 
			", rel_seq: " + std::to_string(rel_seq) +
			", rel_ack: " + std::to_string(rel_ack) + 
			", rel_data: " + std::to_string(rel_data) + 
			", size: " + Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	if (seq <= mIncomingSequence)
	{
		if (Networking::NetLogs >= 1)
		{
			LOG("out of order " + std::to_string(seq) + " packet");
		}
		return;
	}

	if (seq - mIncomingSequence > 1)
	{
		if (Networking::NetLogs >= 1)
		{
			LOG("dropped " + std::to_string(seq - mIncomingSequence - 1) + " packet(s)");
		}
	}

	if (rel_data)
		awake();

	mIncomingSequence = seq;
	mIncomingAcknowledgement = ack;
	
	// TODO: else skip reliable data
	if (rel_seq != mIncomingReliableSequence) // reliable received
	{
		if (Networking::NetLogs >= 2)
			LOG("reliable received");

		if (!rel_data)
			throw std::runtime_error("rel_seq changed without rel_data");

		readReliableDataFromPacket(buf); 
		mIncomingReliableSequence = rel_seq;
	}

	if (rel_ack != mIncomingReliableAcknowledgement) // reliable delivered
	{
		if (Networking::NetLogs >= 2)
			LOG("reliable delivered");
		
		if (!mReliableMessages.empty())
		{
			mReliableMessages.pop_front();
		}
		mIncomingReliableAcknowledgement = rel_ack;
	}

	mIncomingTime = Clock::Now();
}

void Channel::sendReliable(const std::string& msg, Common::BitBuffer& buf)
{
	mReliableMessages.push_back({ msg, std::make_shared<Common::BitBuffer>(buf) });
}

void Channel::addMessageReader(const std::string& msg, ReadCallback callback)
{
	assert(mMessageReaders.count(msg) == 0);
	mMessageReaders.insert({ msg, callback });
}

void Channel::disconnect(const std::string& reason)
{
	mDisconnectCallback(reason);
}

// networking

Networking::Networking(uint16_t port) : mSocket(port)
{
	mSocket.setReadCallback([this](auto& packet) {
		readPacket(packet);
	});

	auto description = "1 - loss, 2 - loss rel in, 3 - loss rel out, 4 - loss rel in out, 5 - loss rel";
	
	CONSOLE->registerCVar("net_logs", description, { "int" },
		CVAR_GETTER_INT(Networking::NetLogs), CVAR_SETTER_INT(Networking::NetLogs));

	CONSOLE->registerCVar("net_reconnect_delay", { "sec" },
		CVAR_GETTER_INT(Networking::NetReconnectDelay), CVAR_SETTER_INT(Networking::NetReconnectDelay));

	CONSOLE->registerCVar("net_timeout", { "sec" },
		CVAR_GETTER_INT(Networking::NetTimeout), CVAR_SETTER_INT(Networking::NetTimeout));

	CONSOLE->registerCVar("net_transmit_duration_min", { "msec" },
		CVAR_GETTER_INT(Networking::NetTransmitDurationMin), CVAR_SETTER_INT(Networking::NetTransmitDurationMin));

	CONSOLE->registerCVar("net_transmit_duration_max", { "msec" },
		CVAR_GETTER_INT(Networking::NetTransmitDurationMax), CVAR_SETTER_INT(Networking::NetTransmitDurationMax));
}

void Networking::readPacket(Network::Packet& packet)
{
	auto msg = packet.buf.readBitsVar();

	if (mMessages.count(msg) == 0)
		throw std::runtime_error(("unknown message type " + std::to_string((int)msg) + " from " + packet.adr.toString()).c_str());

	mMessages.at(msg)(packet);
}

void Networking::addMessage(uint32_t msg, ReadCallback callback)
{
	assert(mMessages.count(msg) == 0);
	mMessages.insert({ msg, callback });
}

void Networking::sendMessage(uint32_t msg, const Network::Address& adr, const Common::BitBuffer& _buf)
{
	auto buf = Common::BitBuffer();
	buf.writeBitsVar(msg);
	buf.write(_buf.getMemory(), _buf.getSize());
	mSocket.sendPacket({ adr, buf });
}

// server

Server::Server(uint16_t port) : Networking(port)
{
	addMessage((uint32_t)Message::Connect, [this](auto& packet) {
		auto adr = packet.adr;

		auto protocol = packet.buf.readBitsVar();

		if (protocol != ProtocolVersion)
			return; // TODO: send something to client to stop him reconnecting

		sendMessage((uint32_t)Message::Connect, adr);

		if (mChannels.count(adr) > 0)
		{
			mChannels.at(adr)->disconnect("reconnect");
			LOG(adr.toString() + " reconnected");
		}
		else
		{
			LOG(adr.toString() + " connected");
		}

		assert(mChannels.count(adr) == 0);

		auto channel = createChannel();
		channel->setSendCallback([this, adr](auto& buf) {
			sendMessage((uint32_t)Message::Regular, adr, buf);
		});
		channel->setDisconnectCallback([this, adr](const auto& reason) {
			LOG(adr.toString() + " disconnected (" + reason + ")");
			mChannels.erase(adr);
		});
		mChannels[adr] = channel;
	});
	addMessage((uint32_t)Message::Regular, [this](auto& packet) {
		if (mChannels.count(packet.adr) == 0)
			return;

		auto channel = mChannels.at(packet.adr);

		try
		{
			channel->read(packet.buf);
		}
		catch (std::exception& e)
		{
			channel->disconnect(e.what());
		}
	});
}

// client

Client::Client(const Network::Address& server_address) :
	mServerAddress(server_address)
{
	addMessage((uint32_t)Networking::Message::Connect, [this](auto& packet) {
		if (mChannel)
			return; // already connected
		
		LOG("connected");

		mChannel = createChannel();
		mChannel->setSendCallback([this](auto& buf) {
			sendMessage((uint32_t)Networking::Message::Regular, mServerAddress, buf);
		});
		mChannel->setDisconnectCallback([this](const auto& reason) {
			mChannel = nullptr;
			LOG("disconnected (" + reason + ")");
		});
	});
	addMessage((uint32_t)Networking::Message::Regular, [this](auto& packet) {
		if (!mChannel)
			return;

		if (packet.adr != mServerAddress)
			return;

		try
		{
			mChannel->read(packet.buf);
		}
		catch (std::exception& e)
		{
			mChannel->disconnect(e.what());
		}
	});

	connect();
}

void Client::frame()
{
	if (isConnected())
		return;

	auto now = Clock::Now();

	if (now - mConnectTime < Clock::FromSeconds(Networking::NetReconnectDelay))
		return;

	connect();
}

void Client::connect()
{
	auto buf = Common::BitBuffer();
	buf.writeBitsVar(ProtocolVersion);
	sendMessage((uint32_t)Networking::Message::Connect, mServerAddress, buf);
	if (Networking::NetLogs >= 1)
	{
		LOG("connecting to " + mServerAddress.toString());
	}
	mConnectTime = Clock::Now();
}
