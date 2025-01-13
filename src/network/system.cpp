#include "system.h"
#include <asio.hpp>

using namespace Network;

struct System::Impl
{
	struct UdpSocketData
	{
		UdpSocketData(asio::io_service& service, const asio::ip::udp::endpoint& endpoint) : socket(service, endpoint) {}
		ReadCallback readCallback = nullptr;
		asio::ip::udp::socket socket;
	};

	std::unordered_set<UdpSocketData*> udp_sockets;
	asio::io_service service;
};

Address::Address()
{
}

Address::Address(const std::string& adr)
{
	auto ip_s = adr;

	auto p = adr.find(":");

	if (p != std::string::npos)
	{
		port = stoi(adr.substr(p + 1));
		ip_s = adr.substr(0, p);
	}
	else
	{
		port = 0;
	}

	try
	{
		auto bytes = asio::ip::address_v4::from_string(ip_s).to_bytes();

		ip.b[0] = bytes[0];
		ip.b[1] = bytes[1];
		ip.b[2] = bytes[2];
		ip.b[3] = bytes[3];
	}
	catch (...)
	{
		auto resolver = asio::ip::udp::resolver(NETWORK->mImpl->service);
		auto query = asio::ip::udp::resolver::query(ip_s, "");
		auto results = resolver.resolve(query);
		for (auto entry : results)
		{
			auto bytes = entry.endpoint().address().to_v4().to_bytes();
			ip.b[0] = bytes[0];
			ip.b[1] = bytes[1];
			ip.b[2] = bytes[2];
			ip.b[3] = bytes[3];
			break;
		}
	}
}

std::string Address::toString() const
{
	return
		std::to_string(ip.b[0]) + "." +
		std::to_string(ip.b[1]) + "." +
		std::to_string(ip.b[2]) + "." +
		std::to_string(ip.b[3]) + ":" +
		std::to_string(port);
}

System::System() :
	mImpl(std::make_unique<Impl>())
{
	mPacketsPerSecondTimer.setInterval(sky::FromSeconds(1.0f));
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
	while (!mImpl->udp_sockets.empty())
		destroyUdpSocket(static_cast<UdpSocketHandle>(*mImpl->udp_sockets.begin()));
}

void System::onFrame()
{
	for (auto socket : mImpl->udp_sockets)
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
	auto socket_data = new Impl::UdpSocketData(mImpl->service, endpoint);

#if defined(PLATFORM_WINDOWS)
	BOOL enable = FALSE;
	DWORD dwBytesRet = 0;
	WSAIoctl(socket_data->socket.native_handle(), SIO_UDP_CONNRESET, &enable, sizeof(enable), NULL, 0, &dwBytesRet, NULL, NULL);
#endif

	mImpl->udp_sockets.insert(socket_data);
	return socket_data;
}

void System::destroyUdpSocket(UdpSocketHandle handle)
{
	auto socket_data = static_cast<Impl::UdpSocketData*>(handle);
	mImpl->udp_sockets.erase(socket_data);
	delete socket_data;
}

void System::sendUdpPacket(UdpSocketHandle handle, const Packet& packet)
{
	auto socket_data = static_cast<Impl::UdpSocketData*>(handle);
	auto buffer = asio::buffer(packet.buf.getMemory(), packet.buf.getSize());
	auto ip = asio::ip::address_v4({ packet.adr.ip.b[0], packet.adr.ip.b[1], packet.adr.ip.b[2], packet.adr.ip.b[3] });
	auto endpoint = asio::ip::udp::endpoint(ip, packet.adr.port);
	socket_data->socket.send_to(buffer, endpoint);
	mOutgoingPacketsCount += 1;
	mOutgoingBytesCount += packet.buf.getSize();
}

void System::setUdpReadCallback(UdpSocketHandle handle, ReadCallback value)
{
	auto socket_data = static_cast<Impl::UdpSocketData*>(handle);
	socket_data->readCallback = value;
}

uint16_t System::getUdpSocketPort(UdpSocketHandle handle) const
{
	auto socket_data = static_cast<Impl::UdpSocketData*>(handle);
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
