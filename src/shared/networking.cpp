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

	if (now - mIncomingTime >= mTimeoutDuration)
	{
		disconnect("timed out");
		return;
	}

	if (!mReliableMessages.empty() && !mIncomingReliableAcknowledgement)
		awake(); // we want send reliable

	auto durationSinceAwake = Clock::ToSeconds(now - mAwakeTime);
	mTransmitDuration = (durationSinceAwake - 0.25f) / 5.0f;
	mTransmitDuration = glm::clamp(mTransmitDuration, 0.0f, 1.0f);

	auto min_duration = Clock::ToSeconds(mTransmitDurationMin);
	auto max_duration = Clock::ToSeconds(mTransmitDurationMax);
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

	bool reliable = !mReliableMessages.empty() && !mIncomingReliableAcknowledgement;

	buf.writeBitsVar(mOutgoingSequence);
	buf.writeBitsVar(mIncomingSequence);
	buf.writeBit(reliable);
	buf.writeBit(mIncomingReliableSequence);

	if (reliable)
	{
		auto& [name, msg] = mReliableMessages.front();
		buf.writeBit(true);
		Common::BufferHelpers::WriteString(buf, name);
		Common::BufferHelpers::WriteToBuffer(*msg, buf);
	}

	buf.writeBit(false);

	if (Networking::NetLogs >= 2)
	{
		LOG("[OUT] seq: " + std::to_string(mOutgoingSequence) + ", ack: " + std::to_string(mIncomingSequence) + ", rel_seq: " + std::to_string(reliable) +
			", rel_ack: " + std::to_string(mIncomingReliableSequence) + ", size: " + Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	mSendCallback(buf);
}

void Channel::awake()
{
	mAwakeTime = Clock::Now();
}

void Channel::read(Common::BitBuffer& buf)
{
	auto seq = buf.readBitsVar();
	auto ack = buf.readBitsVar();
	auto rel_seq = buf.readBit();
	auto rel_ack = buf.readBit();

	if (Networking::NetLogs >= 2)
	{
		LOG("[IN ] seq: " + std::to_string(seq) + ", ack: " + std::to_string(ack) + ", rel_seq: " + std::to_string(rel_seq) +
			", rel_ack: " + std::to_string(rel_ack) + ", size: " + Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	if (seq <= mIncomingSequence)
	{
		if (Networking::NetLogs >= 1)
		{
			LOG("out of order " + std::to_string(seq) + " packet");
		}
		return; // out of order or duplicated packet
	}

	if (seq - mIncomingSequence > 1)
	{
		if (Networking::NetLogs >= 1)
		{
			LOG("dropped " + std::to_string(seq - mIncomingSequence - 1) + " packet(s)");
		}
	}

	mIncomingSequence = seq;
	mIncomingAcknowledgement = ack;

	bool not_duplicated_reliable = !mIncomingReliableSequence && rel_seq;
	mIncomingReliableSequence = rel_seq;

	if (rel_seq)
		awake(); // we want reply every reliable packet

	if (!mIncomingReliableAcknowledgement && rel_ack)
	{
		if (!mReliableMessages.empty())
		{
			mReliableMessages.pop_front();
		}
		else if (Networking::NetLogs >= 1)
		{
			LOG("unexpected reliable acknowledgement!");
		}
		mIncomingReliableAcknowledgement = true;
	}
	else if (mIncomingReliableAcknowledgement && !rel_ack)
	{
		mIncomingReliableAcknowledgement = false;
	}

	if (not_duplicated_reliable)
	{
		while (buf.readBit())
		{
			auto msg = Common::BufferHelpers::ReadString(buf);

			if (mMessageReaders.count(msg) == 0)
				throw std::runtime_error(("unknown message type in channel: " + msg).c_str());

			mMessageReaders.at(msg)(buf);
		}
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

	CONSOLE->registerCVar("net_logs", { "int" }, CVAR_GETTER_INT(Networking::NetLogs), CVAR_SETTER_INT(Networking::NetLogs));
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
			// TODO: test connection overriding and uncomment

			// mChannels.at(adr)->disconnect("reconnect");
			// LOG(adr.toString() + " reconnected");

			return;
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
		LOG("connected");

		assert(!mChannel);
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

	if (now - mConnectTime < mReconnectDelay)
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