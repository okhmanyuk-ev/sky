#include "networking.h"
#include <console/device.h>
#include <common/size_converter.h>
#include <common/buffer_helpers.h>
#include <common/actions.h>

using namespace Shared;

// channel

Channel::Channel()
{
	//
}

void Channel::frame()
{
	auto now = Clock::Now();

	if (now - mIncomingTime >= Clock::FromSeconds(30))
	{
		mTimeoutCallback();
		return;
	}

	if (now - mTransmitTime < Clock::FromMilliseconds(10)) // TODO: make dynamic pps
		return;

	mTransmitTime = now;

	transmit();
}

void Channel::transmit()
{
	mOutgoingSequence += 1;

	auto buf = Common::BitBuffer();

	bool reliable = !mReliableMessages.empty() && mIncomingAcknowledgement >= mReliableSequence;

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

void Channel::read(Common::BitBuffer& buf)
{
	auto seq = buf.readBitsVar();
	auto ack = buf.readBitsVar();
	auto rel_seq = buf.readBit();
	auto rel_ack = buf.readBit();

	if (seq <= mIncomingSequence)
		return; // out of order or duplicated packet

	mIncomingSequence = seq;
	mIncomingAcknowledgement = ack;

	if (rel_seq != mIncomingReliableSequence) // reliable received
		mIncomingReliableSequence = rel_seq;

	if (mIncomingAcknowledgement >= mReliableSequence && rel_ack != mIncomingReliableAcknowledgement) // reliable delivered
	{
		mReliableMessages.pop_front();
		mIncomingReliableAcknowledgement = rel_ack;
	}

	while (buf.readBit())
	{
		auto msg = buf.readBitsVar();

		if (mMessageReaders.count(msg) == 0)
			throw std::exception(("unknown message type in channel: " + std::to_string((int)msg)).c_str());

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
		throw std::exception(("unknown message type " + std::to_string((int)msg) + " from " + packet.adr.toString()).c_str());

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

		LOG(adr.toString() + " connected");

		sendMessage((uint32_t)Message::Connect, adr);

		assert(mChannels.count(adr) == 0);

		auto channel = std::make_shared<Channel>();
		channel->setSendCallback([this, adr](auto& buf) {
			sendMessage((uint32_t)Message::Regular, adr, buf);
		});
		channel->setTimeoutCallback([this, adr] {
			LOG(adr.toString() + " timed out");
			mChannels.erase(adr);
		});
		channel->addMessageReader((uint32_t)Client::Message::Event, [this](auto& buf) {
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
		mChannels[adr] = channel;
	});
	addMessage((uint32_t)Message::Regular, [this](auto& packet) {
		if (mChannels.count(packet.adr) == 0)
			return;

		mChannels.at(packet.adr)->read(packet.buf);
	});
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
		mChannel->setTimeoutCallback([this] {
			mChannel = nullptr;
			LOG("server timed out");
		});
	});
	addMessage((uint32_t)Networking::Message::Regular, [this](auto& packet) {
		if (!mChannel)
			return;

		if (packet.adr != mServerAddress)
			return;

		mChannel->read(packet.buf);
	});

	sendMessage((uint32_t)Networking::Message::Connect, mServerAddress);
	LOG("connecting to " + mServerAddress.toString());
}

void Client::sendEvent(const std::string& name, const std::map<std::string, std::string>& params)
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

	mChannel->sendReliable((uint32_t)Message::Event, buf);
}