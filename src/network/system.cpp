#include "system.h"
#include <stdexcept>

#if defined(PLATFORM_WINDOWS)
#pragma comment(lib,"ws2_32.lib")
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

using namespace Network;

#if defined(PLATFORM_WINDOWS)
#define close closesocket
#define socklen_t int
#endif

#if defined(PLATFORM_ANDROID) | defined(PLATFORM_IOS)
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif

System::System()
{
#if defined(PLATFORM_WINDOWS)
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

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

#if defined(PLATFORM_WINDOWS)
	WSACleanup();
#endif
}

void System::frame()
{
	sockaddr_in adr;
	socklen_t adr_size = sizeof(adr);
	
	for (auto socket : mUdpSockets)
	{
		while (true)
		{
			int size = recvfrom(socket->socket, mBuffer, BufferSize, 0, (sockaddr*)&adr, &adr_size);

			if (size == -1)
				break;

			Packet packet;

			packet.adr.ip.l = adr.sin_addr.s_addr;
			packet.adr.port = ntohs(adr.sin_port);

			packet.buf.write(mBuffer, size);
			packet.buf.toStart();

			if (socket->readCallback)
				socket->readCallback(packet);

			mIncomingPacketsCount += 1;
			mIncomingBytesCount += size;
		}
	}
}

void System::throwLastError()
{
	auto err = errno;
#if defined(PLATFORM_WINDOWS)
	err = GetLastError();
#endif
	throw std::runtime_error("socket error: " + std::to_string(err));
}

System::UdpSocketHandle System::createUdpSocket(uint16_t port)
{
	auto socket_data = new UdpSocketData;

	socket_data->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (socket_data->socket == INVALID_SOCKET)
		throwLastError();
	
	sockaddr_in adr;
	socklen_t adr_size = sizeof(adr);
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(port);

	if (bind(socket_data->socket, (sockaddr*)&adr, adr_size) == SOCKET_ERROR)
		throwLastError();
	
	if (getsockname(socket_data->socket, (sockaddr*)&adr, &adr_size) == SOCKET_ERROR)
		throwLastError();
	
	socket_data->port = ntohs(adr.sin_port);

#if defined(PLATFORM_WINDOWS)
	u_long tr = 1;
	if (ioctlsocket(socket_data->socket, FIONBIO, &tr) == SOCKET_ERROR)
		throwLastError();
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (fcntl(socket_data->socket, F_SETFL, fcntl(socket_data->socket, F_GETFL) | O_NONBLOCK) == SOCKET_ERROR)
		throwLastError();
#endif

	mUdpSockets.insert(socket_data);
	return socket_data;
}

void System::destroyUdpSocket(UdpSocketHandle handle)
{
	auto socket_data = static_cast<UdpSocketData*>(handle);
	close(socket_data->socket);
	mUdpSockets.erase(socket_data);
	delete socket_data;
}

void System::sendUdpPacket(UdpSocketHandle handle, const Packet& packet)
{
	auto socket_data = static_cast<UdpSocketData*>(handle);

	sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = packet.adr.ip.l;
	adr.sin_port = htons(packet.adr.port);

	sendto(socket_data->socket, (const char*)packet.buf.getMemory(), packet.buf.getSize(), 0,
		(sockaddr*)&adr, sizeof(adr));

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
	return socket_data->port;
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

auto UdpSocket::getPort() const
{
	return NETWORK->getUdpSocketPort(mHandle);
}