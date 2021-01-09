#include "server.h"
#include <console/device.h>
#include <common/size_converter.h>

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

void Networking::sendPacket(const Network::Packet& packet)
{
	mSocket.sendPacket(packet);
}

// server

Server::Server(uint16_t port) : Networking(port)
{
	addMessage((uint32_t)ClientMessage::Connect, [this](auto& packet) {
		LOG(packet.adr.toString() + " connected");

		auto buf = Common::BitBuffer();
		buf.writeBitsVar((uint32_t)ServerMessage::ConnectAccepted);
		sendPacket({ packet.adr, buf });
	});
}

// client

Client::Client(const Network::Address& server_address) :
	mServerAddress(server_address)
{
	addMessage((uint32_t)ServerMessage::ConnectAccepted, [this](auto& packet) {
		LOG("connection accepted");
	});

	auto buf = Common::BitBuffer();
	buf.writeBitsVar((uint32_t)ClientMessage::Connect);
	sendPacket({ mServerAddress, buf });
}