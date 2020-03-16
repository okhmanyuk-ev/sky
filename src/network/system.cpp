#include "system.h"

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

System::System()
{
#if defined(PLATFORM_WINDOWS)
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

System::~System()
{
	while (!mSockets.empty())
		destroySocket(static_cast<SocketHandle>(*mSockets.begin()));

#if defined(PLATFORM_WINDOWS)
	WSACleanup();
#endif
}

void System::frame()
{
#if defined(PLATFORM_WINDOWS)
	SOCKADDR_IN adr;
	int adr_size = sizeof(adr);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sockaddr_in adr;
	socklen_t adr_size = sizeof(adr);
#endif

	for (auto socket : mSockets)
	{
		while (true)
		{
#if defined(PLATFORM_WINDOWS)
			int size = recvfrom(socket->socket, mBuffer, 8192, 0, (SOCKADDR*)&adr, &adr_size);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
			int size = recvfrom(socket->socket, mBuffer, 8192, 0, (sockaddr*)&adr, &adr_size);
#endif
			if (size == -1)
				break;

			Packet packet;

			packet.adr.ip.l = adr.sin_addr.s_addr;
			packet.adr.port = ntohs(adr.sin_port);

			packet.buf.write(mBuffer, size);

			if (socket->readCallback)
				socket->readCallback(packet);
		}
	}
}

System::SocketHandle System::createSocket(uint16_t port)
{
	auto socket_data = new SocketData;

	socket_data->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

#if defined(PLATFORM_WINDOWS)
	if (socket_data->socket == INVALID_SOCKET)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (socket_data->socket == -1)
#endif
	{
		//	auto a = errno;
		//	throw std::runtime_error("error while creating socket");
	}

#if defined(PLATFORM_WINDOWS)
	SOCKADDR_IN adr;
	int adr_size = sizeof(adr);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sockaddr_in adr;
	socklen_t adr_size = sizeof(adr);
#endif
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(port);


#if defined(PLATFORM_WINDOWS)
	if (bind(socket_data->socket, (SOCKADDR*)&adr, adr_size) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (bind(socket_data->socket, (sockaddr*)&adr, adr_size) == -1)
#endif
	{
		//	throw std::runtime_error("error while binding socket");
	}

#if defined(PLATFORM_WINDOWS)
	if (getsockname(socket_data->socket, (SOCKADDR*)&adr, &adr_size) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (getsockname(socket_data->socket, (sockaddr*)&adr, &adr_size) == -1)
#endif
	{
		//	throw std::runtime_error("error while getting sock port");
	}

	socket_data->port = ntohs(adr.sin_port);

	u_long tr = 1;

#if defined(PLATFORM_WINDOWS)
	if (ioctlsocket(socket_data->socket, FIONBIO, &tr) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (fcntl(socket_data->socket, F_SETFL, fcntl(socket_data->socket, F_GETFL) | O_NONBLOCK) == -1)
#endif
	{
		//	throw std::runtime_error("cannot set socket blocking state");
	}

	mSockets.insert(socket_data);
	return socket_data;
}

void System::destroySocket(SocketHandle handle)
{
	auto socket_data = static_cast<SocketData*>(handle);

#if defined(PLATFORM_WINDOWS)
	closesocket(socket_data->socket);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	close(socket_data->socket);
#endif

	mSockets.erase(socket_data);
	delete socket_data;
}

void System::sendPacket(SocketHandle handle, const Packet& packet)
{
	auto socket_data = static_cast<SocketData*>(handle);

#if defined(PLATFORM_WINDOWS)
	SOCKADDR_IN adr;
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sockaddr_in adr;
#endif
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = packet.adr.ip.l;
	adr.sin_port = htons(packet.adr.port);

#if defined(PLATFORM_WINDOWS)
	sendto(socket_data->socket, (const char*)packet.buf.getMemory(), packet.buf.getSize(), 0,
		(SOCKADDR*)&adr, sizeof(adr));
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sendto(socket_data->socket, (const char*)packet.buf.getMemory(), packet.buf.getSize(), 0,
		(sockaddr*)&adr, sizeof(adr));
#endif
}

void System::setReadCallback(SocketHandle handle, ReadCallback value)
{
	auto socket_data = static_cast<SocketData*>(handle);
	socket_data->readCallback = value;
}

uint64_t System::getPort(SocketHandle handle) const
{
	auto socket_data = static_cast<SocketData*>(handle);
	return socket_data->port;
}

Socket::Socket(uint64_t port)
{
	mHandle = NETWORK->createSocket(port);
}

Socket::~Socket()
{
	NETWORK->destroySocket(mHandle);
}

void Socket::sendPacket(const Packet& packet)
{
	NETWORK->sendPacket(mHandle, packet);
}

void Socket::setReadCallback(System::ReadCallback value)
{
	NETWORK->setReadCallback(mHandle, value);
}

auto Socket::getPort() const
{
	return NETWORK->getPort(mHandle);
}