#include "networking_udp.h"

#include <console/device.h>
#include <common/buffer_helpers.h>
#include <common/helpers.h>
#include <common/console_commands.h>

using namespace Shared::NetworkingUDP;

// channel

void Channel::onFrame()
{
	if (mDisconnect.has_value())
	{
		mDisconnectCallback(mDisconnect.value());
		return;
	}

	auto now = Clock::Now();

	if (now - mIncomingTime >= Clock::FromSeconds(Networking::NetTimeout))
	{
		disconnect("timed out");
		return;
	}

	if (!mOutgoingReliableMessages.empty())
		awake();

	if (!mReliableAcknowledgements.empty())
		awake();

	auto durationSinceAwake = Clock::ToSeconds(now - mAwakeTime);
	mHibernation = (durationSinceAwake - 0.5f) / 10.0f;
	mHibernation = glm::clamp(mHibernation, 0.0f, 1.0f);

	auto min_delay = Clock::ToSeconds(Clock::FromMilliseconds(Networking::NetTransmitDelayMin));
	auto max_delay = Clock::ToSeconds(Clock::FromMilliseconds(Networking::NetTransmitDelayMax));
	auto transmit_delay = Clock::FromSeconds(glm::lerp(min_delay, max_delay, mHibernation));

	if (now - mTransmitTime < transmit_delay)
		return;

	mTransmitTime = now;

	transmit();
}

void Channel::transmit()
{
	mOutgoingSequence += 1;
	
	auto buf = BitBuffer();

	buf.writeBitsVar(mOutgoingSequence);
	buf.writeBitsVar(mIncomingSequence);
	
	for (auto index : mReliableAcknowledgements)
	{
		buf.writeBit(true);
		buf.writeBitsVar(index);
	}

	buf.writeBit(false);

	mReliableAcknowledgements.clear();
	
	while (!mOutgoingReliableMessages.empty())
	{
		auto [index, rel_msg] = *mOutgoingReliableMessages.begin();

		if (buf.getSize() + rel_msg.buf->getSize() > Networking::NetMaxPacketSize)
		{
			// TODO: if rel_msg size is too much it will deadlock here (we should make fragmented channel)
			break;
		}

		buf.writeBit(true);
		buf.writeBitsVar(index);
		Common::BufferHelpers::WriteString(buf, rel_msg.name);

		auto size = rel_msg.buf->getSize(); // TODO: use bit corrected size
		buf.writeBitsVar(size);
		buf.write(rel_msg.buf->getMemory(), size);

		mPendingOutgoingReliableMessages.insert({ index, { mOutgoingSequence, rel_msg } });
		mOutgoingReliableMessages.erase(index);

		if (Networking::NetLogRel)
			sky::Log("write reliable {}", index);
	}

	buf.writeBit(false);

	// TODO: send unreliable data here

	if (Networking::NetLogPackets)
	{
		sky::Log("[OUT] seq: {}, ack: {}, size: {}", mOutgoingSequence, mIncomingSequence,
			Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	mSendCallback(buf);
}

void Channel::awake()
{
	mAwakeTime = Clock::Now();
}

void Channel::readReliableMessages()
{
	while (mIncomingReliableMessages.count(mIncomingReliableIndex + 1))
	{
		mIncomingReliableIndex += 1;

		auto [name, rel_buf] = mIncomingReliableMessages.at(mIncomingReliableIndex);

		if (Networking::NetLogRel)
			sky::Log("read reliable {}", mIncomingReliableIndex);

		if (mMessageReaders.count(name) == 0)
			throw std::runtime_error(("unknown message type in channel: " + name).c_str());

		mMessageReaders.at(name)(*rel_buf);
		mIncomingReliableMessages.erase(mIncomingReliableIndex);
	}
}

void Channel::resendReliableMessages(uint32_t ack)
{
	for (auto [index, pending] : mPendingOutgoingReliableMessages)
	{
		if (ack < pending.sequence)
			continue;

		mOutgoingReliableMessages.insert({ index, pending.rel_msg });
		mPendingOutgoingReliableMessages.erase(index);

		if (Networking::NetLogRel)
			sky::Log("resend reliable {}", index);
		
		resendReliableMessages(ack);
		return;
	}
}

void Channel::read(BitBuffer& buf)
{
	auto seq = buf.readBitsVar();
	auto ack = buf.readBitsVar();

	if (seq <= mIncomingSequence)
	{
		if (Networking::NetLogLoss)
			sky::Log("out of order {} packet", seq);
		
		return;
	}

	if (seq - mIncomingSequence > 1)
	{
		if (Networking::NetLogLoss)
			sky::Log("dropped {} packet(s)", seq - mIncomingSequence - 1);
	}

	mIncomingSequence = seq;

	while (buf.readBit())
	{
		auto index = buf.readBitsVar();
		mPendingOutgoingReliableMessages.erase(index);
		
		if (Networking::NetLogRel)
			sky::Log("reliable {} delivered", index);
	}

	while (buf.readBit())
	{
		auto index = buf.readBitsVar();

		mReliableAcknowledgements.insert(index);

		auto msg = ReliableMessage();
		msg.name = Common::BufferHelpers::ReadString(buf);
		msg.buf = std::make_shared<BitBuffer>();

		auto size = buf.readBitsVar(); // TODO: use bit corrected size

		for (uint32_t i = 0; i < size; i++)
			msg.buf->write(buf.read<uint8_t>());

		msg.buf->toStart();

		if (mIncomingReliableIndex >= index)
		{
			if (Networking::NetLogRel)
				sky::Log("reliable {} was already received and readed", index);

			continue;
		}

		if (mIncomingReliableMessages.count(index) > 0)
		{
			if (Networking::NetLogRel)
				sky::Log("reliable {} was already received but not yet readed", index);

			continue;
		}
		mIncomingReliableMessages.insert({ index, msg });

		if (Networking::NetLogRel)
			sky::Log("reliable {} received", index);
	}

	readReliableMessages();
	resendReliableMessages(ack);

	if (Networking::NetLogPackets)
	{
		sky::Log("[IN ] seq: {}, ack: {}, size: {}", seq, ack,
			Common::Helpers::BytesToNiceString(buf.getSize()));
	}

	mIncomingTime = Clock::Now();
}

void Channel::sendReliable(const std::string& msg, BitBuffer& buf)
{
	mOutgoingReliableIndex += 1;
	mOutgoingReliableMessages.insert({ mOutgoingReliableIndex, { msg, std::make_shared<BitBuffer>(buf) } });
	
	if (Networking::NetLogRel)
		sky::Log("send reliable {}", mOutgoingReliableIndex);
}

void Channel::addMessageReader(const std::string& msg, ReadCallback callback)
{
	assert(mMessageReaders.count(msg) == 0);
	mMessageReaders.insert({ msg, callback });
}

void Channel::disconnect(const std::string& reason)
{
	if (mDisconnect.has_value())
		return;

	mDisconnect = reason;
}

// networking

Networking::Networking(uint16_t port) : mSocket(port)
{
	mSocket.setReadCallback([this](auto& packet) {
		readPacket(packet);
	});

	CONSOLE->registerCVar("net_log_packets", { "bool" },
		CVAR_GETTER_BOOL(Networking::NetLogPackets), CVAR_SETTER_BOOL(Networking::NetLogPackets));

	CONSOLE->registerCVar("net_log_loss", { "bool" },
		CVAR_GETTER_BOOL(Networking::NetLogLoss), CVAR_SETTER_BOOL(Networking::NetLogLoss));

	CONSOLE->registerCVar("net_log_rel", { "bool" },
		CVAR_GETTER_BOOL(Networking::NetLogRel), CVAR_SETTER_BOOL(Networking::NetLogRel));

	CONSOLE->registerCVar("net_reconnect_delay", { "sec" },
		CVAR_GETTER_INT(Networking::NetReconnectDelay), CVAR_SETTER_INT(Networking::NetReconnectDelay));

	CONSOLE->registerCVar("net_timeout", { "sec" },
		CVAR_GETTER_INT(Networking::NetTimeout), CVAR_SETTER_INT(Networking::NetTimeout));

	CONSOLE->registerCVar("net_transmit_delay_min", { "msec" },
		CVAR_GETTER_INT(Networking::NetTransmitDelayMin), CVAR_SETTER_INT(Networking::NetTransmitDelayMin));

	CONSOLE->registerCVar("net_transmit_delay_max", { "msec" },
		CVAR_GETTER_INT(Networking::NetTransmitDelayMax), CVAR_SETTER_INT(Networking::NetTransmitDelayMax));

	CONSOLE->registerCVar("net_max_packet_size", { "bytes" },
		CVAR_GETTER_INT(Networking::NetMaxPacketSize), CVAR_SETTER_INT(Networking::NetMaxPacketSize));
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

void Networking::sendMessage(uint32_t msg, const Network::Address& adr, const BitBuffer& _buf)
{
	auto buf = BitBuffer();
	buf.writeBitsVar(msg);
	buf.write(_buf.getMemory(), _buf.getSize());
	mSocket.sendPacket({ adr, buf });
}

void Networking::sendDisconnect(const Network::Address& address, const std::string& reason)
{
	auto buf = BitBuffer();
	Common::BufferHelpers::WriteString(buf, reason);
	sendMessage((uint32_t)Message::Disconnect, address, buf);
}

void Networking::sendRedirect(const Network::Address& address, const std::string& redirect_address)
{
	auto buf = BitBuffer();
	Common::BufferHelpers::WriteString(buf, redirect_address);
	sendMessage((uint32_t)Message::Redirect, address, buf);
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
			sky::Log(adr.toString() + " reconnected");
		}
		else
		{
			sky::Log(adr.toString() + " connected");
		}

		assert(mChannels.count(adr) == 0);

		auto channel = createChannel();
		channel->setSendCallback([this, adr](auto& buf) {
			sendMessage((uint32_t)Message::Regular, adr, buf);
		});
		channel->setDisconnectCallback([this, adr](const auto& reason) {
			sendDisconnect(adr, reason);
			sky::Log("{} disconnected ({})", adr.toString(), reason);
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
	addMessage((uint32_t)Message::Disconnect, [this](auto& packet) {
		if (mChannels.count(packet.adr) == 0)
			return;

		auto reason = Common::BufferHelpers::ReadString(packet.buf);
		
		sky::Log("{} disconnected ({})", packet.adr.toString(), reason);
		mChannels.erase(packet.adr);
	});
}

// client

Client::Client(const Network::Address& server_address) :
	mServerAddress(server_address)
{
	addMessage((uint32_t)Networking::Message::Connect, [this](auto& packet) {
		if (mChannel)
			return; // already connected

		sky::Log("connected");

		mChannel = createChannel();
		mChannel->setSendCallback([this](auto& buf) {
			sendMessage((uint32_t)Networking::Message::Regular, mServerAddress, buf);
		});
		mChannel->setDisconnectCallback([this](const auto& reason) {
			sendDisconnect(mServerAddress, reason);
			sky::Log("disconnected ({})", reason);
			mChannel = nullptr;
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
	addMessage((uint32_t)Message::Disconnect, [this](auto& packet) {
		if (packet.adr != mServerAddress)
			return;

		if (!mChannel)
			return;

		auto reason = Common::BufferHelpers::ReadString(packet.buf);
		
		mChannel = nullptr;
		sky::Log("disconnected ({})", reason);
	});
	addMessage((uint32_t)Message::Redirect, [this](auto& packet) {
		if (packet.adr != mServerAddress)
			return; 
		
		auto redirect_address = Common::BufferHelpers::ReadString(packet.buf);
		
		sky::Log("redirected to {}", redirect_address);
		mServerAddress = redirect_address;
		connect();
	});

	connect();
}

void Client::onFrame()
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
	auto buf = BitBuffer();
	buf.writeBitsVar(ProtocolVersion);
	sendMessage((uint32_t)Networking::Message::Connect, mServerAddress, buf);
	sky::Log("connecting to " + mServerAddress.toString());
	mConnectTime = Clock::Now();
}

// simplechannel

SimpleChannel::SimpleChannel()
{
	addMessageReader("event", [this](auto& buf) { onEventMessage(buf); });
}

void SimpleChannel::sendEvent(const std::string& name, const std::map<std::string, std::string>& params)
{
	auto buf = BitBuffer();
	Common::BufferHelpers::WriteString(buf, name);
	for (auto& [key, value] : params)
	{
		buf.writeBit(true);
		Common::BufferHelpers::WriteString(buf, key);
		Common::BufferHelpers::WriteString(buf, value);
	}
	buf.writeBit(false);
	sendReliable("event", buf);
}

void SimpleChannel::onEventMessage(BitBuffer& buf)
{
	auto name = Common::BufferHelpers::ReadString(buf);
	auto params = std::map<std::string, std::string>();
	while (buf.readBit())
	{
		auto key = Common::BufferHelpers::ReadString(buf);
		auto value = Common::BufferHelpers::ReadString(buf);
		params.insert({ key, value });
	}
	if (mShowEventLogs || mEvents.count(name) == 0)
	{
		sky::Log("event: \"" + name + "\"");
		for (const auto& [key, value] : params)
		{
			sky::Log(" - {} : {}", key, value);
		}
	}
	if (mEvents.count(name) > 0)
	{
		mEvents.at(name)(params);
	}
}

void SimpleChannel::addEventCallback(const std::string& name, EventCallback callback)
{
	assert(mEvents.count(name) == 0);
	mEvents[name] = callback;
}
