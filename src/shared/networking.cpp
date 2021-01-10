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

Channel::Channel()
{
	//CONSOLE->registerCommand("asd", [this](CON_ARGS) {
	//	mIncomingReliableAcknowledgement = !mIncomingReliableAcknowledgement;
	//});
}

void Channel::frame()
{
	auto now = Clock::Now();

	if (now - mIncomingTime >= mTimeoutDuration)
	{
		disconnect("timed out");
		return;
	}

	if (!mMessageWriters.empty())
		awake(); // we want stream data with loss

	if (!mReliableMessages.empty() && !awaitingReliableAcknowledgement())
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

	bool reliable = !mReliableMessages.empty() && !awaitingReliableAcknowledgement();

	if (reliable)
	{
		mOutgoingReliableSequence = !mIncomingReliableAcknowledgement;
		mReliableSequence = mOutgoingSequence;
	}

	buf.writeBitsVar(mOutgoingSequence);
	buf.writeBitsVar(mIncomingSequence);
	buf.writeBit(mOutgoingReliableSequence);
	buf.writeBit(mIncomingReliableSequence);

	if (reliable)
	{
		auto [msg, _buf] = mReliableMessages.front();
		buf.writeBit(true);
		buf.writeBitsVar(msg);
		Common::BufferHelpers::WriteToBuffer(*_buf, buf);
	}

	for (auto& [msg, callback] : mMessageWriters)
	{
		buf.writeBit(true);
		buf.writeBitsVar(msg);
		callback(buf);
	}

	buf.writeBit(false);

	mSendCallback(buf);
}

void Channel::awake()
{
	mAwakeTime = Clock::Now();
}

bool Channel::awaitingReliableAcknowledgement() const
{
	return mIncomingAcknowledgement < mReliableSequence;
}

void Channel::read(Common::BitBuffer& buf)
{
	auto seq = buf.readBitsVar();
	auto ack = buf.readBitsVar();
	auto rel_seq = buf.readBit();
	auto rel_ack = buf.readBit();

	if (seq <= mIncomingSequence)
	{
		LOG("out of order " + std::to_string(seq - mIncomingSequence) + " packet(s)"); // TODO: del
		return; // out of order or duplicated packet
	}

	if (seq - mIncomingSequence > 1)
		LOG("dropped " + std::to_string(seq - mIncomingSequence) + " packet(s)"); // TODO: del

	mIncomingSequence = seq;
	mIncomingAcknowledgement = ack;

	if (rel_seq != mIncomingReliableSequence) // reliable received
	{
		mIncomingReliableSequence = rel_seq;
		awake(); // we want answer as soon as possible
	}

	if (!awaitingReliableAcknowledgement() && rel_ack != mIncomingReliableAcknowledgement) // reliable delivered
	{
		if (!mReliableMessages.empty())
		{
			mReliableMessages.pop_front();
		}
		mIncomingReliableAcknowledgement = rel_ack;
	}

	while (buf.readBit())
	{
		auto msg = buf.readBitsVar();

		if (mMessageReaders.count(msg) == 0)
			throw std::runtime_error(("unknown message type in channel: " + std::to_string((int)msg)).c_str());

		mMessageReaders.at(msg)(buf);
	}

	//LOG("seq: " + std::to_string(seq) + ", ack: " + std::to_string(ack) +
	//	", rel_seq: " + std::to_string(rel_seq) + ", rel_ack: " + std::to_string(rel_ack));

	mIncomingTime = Clock::Now();
}

void Channel::sendReliable(uint32_t msg, Common::BitBuffer& buf)
{
	mReliableMessages.push_back({ msg, std::make_shared<Common::BitBuffer>(buf) });
}

void Channel::addMessageReader(uint32_t msg, ReadCallback callback)
{
	assert(mMessageReaders.count(msg) == 0);
	mMessageReaders.insert({ msg, callback });
}

void Channel::addMessageWriter(uint32_t msg, WriteCallback callback)
{
	assert(mMessageWriters.count(msg) == 0);
	mMessageWriters.insert({ msg, callback });
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

// server channel

Server::Channel::Channel()
{
	addMessageReader((uint32_t)Client::Message::Event, [this](auto& buf) {
		auto name = Common::BufferHelpers::ReadString(buf);
		auto params = std::map<std::string, std::string>();
		while (buf.readBit())
		{
			auto key = Common::BufferHelpers::ReadString(buf);
			auto value = Common::BufferHelpers::ReadString(buf);
			params.insert({ key, value });
		}
		onEvent(name, params);
	});
}

void Server::Channel::sendEvent(const std::string& name, const std::map<std::string, std::string>& params)
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

	sendReliable((uint32_t)Message::Event, buf);
}

// client

Client::Client(const Network::Address& server_address) :
	mServerAddress(server_address)
{
	addMessage((uint32_t)Networking::Message::Connect, [this](auto& packet) {
		LOG("connected");

		assert(!mChannel);
		mChannel = std::make_shared<Channel>();
		mChannel->setSendCallback([this](auto& buf) {
			sendMessage((uint32_t)Networking::Message::Regular, mServerAddress, buf);
		});
		mChannel->setDisconnectCallback([this](const auto& reason) {
			mChannel = nullptr;
			LOG("disconnected (" + reason + ")");
		});
		mChannel->addMessageReader((uint32_t)Client::Message::Event, [this](auto& buf) {
			auto name = Common::BufferHelpers::ReadString(buf);
			auto params = std::map<std::string, std::string>();
			while (buf.readBit())
			{
				auto key = Common::BufferHelpers::ReadString(buf);
				auto value = Common::BufferHelpers::ReadString(buf);
				params.insert({ key, value });
			}
			onEvent(name, params);
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
	// LOG("connecting to " + mServerAddress.toString());
	mConnectTime = Clock::Now();
}

void Client::sendEvent(const std::string& name, const std::map<std::string, std::string>& params)
{
	if (!mChannel)
		return; // TODO: what we should do in this case ?

	auto buf = Common::BitBuffer();
	Common::BufferHelpers::WriteString(buf, name);

	for (auto& [key, value] : params)
	{
		buf.writeBit(true);
		Common::BufferHelpers::WriteString(buf, key);
		Common::BufferHelpers::WriteString(buf, value);
	}

	buf.writeBit(false);

	mChannel->sendReliable((uint32_t)Message::Event, buf);
}