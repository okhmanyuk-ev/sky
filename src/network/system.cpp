#include "system.h"
#include <console/device.h>

using namespace Network;

System::System()
{
	mPacketsPerSecondTimer.setInterval(Clock::FromSeconds(1.0f));
	mPacketsPerSecondTimer.setCallback([this] {
		mIncomingPacketsPerSecond = mIncomingPacketsCount - mPrevIncomingPacketsPerSecond;
		mOutgoingPacketsPerSecond = mOutgoingPacketsCount - mPrevOutgoingPacketsPerSecond;
		mPrevIncomingPacketsPerSecond = mIncomingPacketsCount;
		mPrevOutgoingPacketsPerSecond = mOutgoingPacketsCount;

		mIncomingBytesPerSecond = mIncomingBytesCount - mPrevIncomingBytesPerSecond;
		mOutgoingBytesPerSecond = mOutgoingBytesCount - mPrevOutgoingBytesPerSecond;
		mPrevIncomingBytesPerSecond = mIncomingBytesCount;
		mPrevOutgoingBytesPerSecond = mOutgoingBytesCount;
	});
}

System::~System()
{
	while (!mUdpSockets.empty())
		destroyUdpSocket(static_cast<UdpSocketHandle>(*mUdpSockets.begin()));
}

void System::frame()
{
	for (auto socket : mUdpSockets)
	{
		while (true)
		{
			auto available = socket->socket.available();

			if (available == 0)
				break;

			Packet packet;

			asio::ip::udp::endpoint endpoint;

			packet.buf.setSize(available);

			auto real_size = socket->socket.receive_from(asio::buffer(packet.buf.getMemory(), packet.buf.getSize()), endpoint);
			packet.buf.setSize(real_size);
			auto ip = endpoint.address().to_v4().to_bytes();

			packet.adr.ip.b[0] = ip[0];
			packet.adr.ip.b[1] = ip[1];
			packet.adr.ip.b[2] = ip[2];
			packet.adr.ip.b[3] = ip[3];
			packet.adr.port = endpoint.port();

			mIncomingPacketsCount += 1;
			mIncomingBytesCount += packet.buf.getSize();

			if (socket->readCallback)
				socket->readCallback(packet);
		}
	}
}

System::UdpSocketHandle System::createUdpSocket(uint16_t port)
{
	auto endpoint = asio::ip::udp::endpoint(asio::ip::address_v4::any(), port);
	auto socket_data = new UdpSocketData(mService, endpoint);

#if defined(PLATFORM_WINDOWS)
	BOOL enable = FALSE;
	DWORD dwBytesRet = 0;
	WSAIoctl(socket_data->socket.native_handle(), SIO_UDP_CONNRESET, &enable, sizeof(enable), NULL, 0, &dwBytesRet, NULL, NULL);
#endif

	mUdpSockets.insert(socket_data);
	return socket_data;
}

void System::destroyUdpSocket(UdpSocketHandle handle)
{
	auto socket_data = static_cast<UdpSocketData*>(handle);
	mUdpSockets.erase(socket_data);
	delete socket_data;
}

void System::sendUdpPacket(UdpSocketHandle handle, const Packet& packet)
{
	auto socket_data = static_cast<UdpSocketData*>(handle);
	auto buffer = asio::buffer(packet.buf.getMemory(), packet.buf.getSize());
	auto ip = asio::ip::address_v4({ packet.adr.ip.b[0], packet.adr.ip.b[1], packet.adr.ip.b[2], packet.adr.ip.b[3] });
	auto endpoint = asio::ip::udp::endpoint(ip, packet.adr.port);
	socket_data->socket.send_to(buffer, endpoint);
	mOutgoingPacketsCount += 1;
	mOutgoingBytesCount += packet.buf.getSize();
}

void System::setUdpReadCallback(UdpSocketHandle handle, ReadCallback value)
{
	auto socket_data = static_cast<UdpSocketData*>(handle);
	socket_data->readCallback = value;
}

uint64_t System::getUdpSocketPort(UdpSocketHandle handle) const
{
	auto socket_data = static_cast<UdpSocketData*>(handle);
	return socket_data->socket.local_endpoint().port();
}

UdpSocket::UdpSocket(uint16_t port)
{
	mHandle = NETWORK->createUdpSocket(port);
}

UdpSocket::~UdpSocket()
{
	NETWORK->destroyUdpSocket(mHandle);
}

void UdpSocket::sendPacket(const Packet& packet)
{
	NETWORK->sendUdpPacket(mHandle, packet);
}

void UdpSocket::setReadCallback(System::ReadCallback value)
{
	NETWORK->setUdpReadCallback(mHandle, value);
}

uint16_t UdpSocket::getPort() const
{
	return NETWORK->getUdpSocketPort(mHandle);
}