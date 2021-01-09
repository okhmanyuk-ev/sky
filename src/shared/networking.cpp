#include "networking.h"
#include <console/device.h>
#include <common/size_converter.h>
#include <common/buffer_helpers.h>
#include <common/actions.h>

using namespace Shared;

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
	{
		LOG("unknown message type " + std::to_string((int)msg) + " from " + packet.adr.toString());
		return;
	}

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

// channel

Channel::Channel()
{
	//
}

void Channel::frame()
{
	auto now = Clock::Now();

	if (now - mIncomingTime >= Clock::FromSeconds(5))
	{
		mTimeoutCallback();
		return;
	}

	if (now - mTransmitTime < Clock::FromMilliseconds(1000))
		return;

	mTransmitTime = now;

	transmit();
}

void Channel::transmit()
{
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

	mOutgoingSequence += 1;

	if (reliable)
	{
		Common::BufferHelpers::WriteToBuffer(*mReliableMessages.front(), buf);
	}

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

	mIncomingTime = Clock::Now();
}

void Channel::sendReliable(Common::BitBuffer& buf)
{
	mReliableMessages.push_back(std::make_shared<Common::BitBuffer>(buf));
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
	addMessage((uint32_t)Message::Connect, [this](auto& packet) {
		LOG("connected");

		assert(!mChannel);
		mChannel = std::make_shared<Channel>();
		mChannel->setSendCallback([this](auto& buf) {
			sendMessage((uint32_t)Message::Regular, mServerAddress, buf);
		});
		mChannel->setTimeoutCallback([this] {
			mChannel = nullptr;
			LOG("server timed out");
		});
	});
	addMessage((uint32_t)Message::Regular, [this](auto& packet) {
		if (packet.adr != mServerAddress)
			return;

		mChannel->read(packet.buf);
	});

	sendMessage((uint32_t)Message::Connect, mServerAddress);
	LOG("connecting to " + mServerAddress.toString());
}