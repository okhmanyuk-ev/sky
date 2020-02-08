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

void Socket::sendPacket(const Packet& packet)
{
#if defined(PLATFORM_WINDOWS)
	SOCKADDR_IN adr;
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sockaddr_in adr;
#endif
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = packet.adr.ip.l;
	adr.sin_port = htons(packet.adr.port);

#if defined(PLATFORM_WINDOWS)
	sendto(mSocket, (const char*)packet.buf.getMemory(), packet.buf.getSize(), 0,
		(SOCKADDR*)&adr, sizeof(adr));
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	sendto(mSocket, (const char*)packet.buf.getMemory(), packet.buf.getSize(), 0,
			(sockaddr*)&adr, sizeof(adr));
#endif
}

System::System()
{
#if defined(PLATFORM_WINDOWS)
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

System::~System()
{
	while (mSockets.size() > 0)
		destroySocket(mSockets.front());

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
			int size = recvfrom(socket->mSocket, mBuffer, 8192, 0, (SOCKADDR*)&adr, &adr_size);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
			int size = recvfrom(socket->mSocket, mBuffer, 8192, 0, (sockaddr*)&adr, &adr_size);
#endif
			if (size == -1)
				break;

			Packet packet;

			packet.adr.ip.l = adr.sin_addr.s_addr;
			packet.adr.port = ntohs(adr.sin_port);

			packet.buf.write(mBuffer, size);

			if (socket->mReadCallback)
				socket->mReadCallback(packet);
		}
	}
}

Socket* System::createSocket(uint16_t port) // TODO: make good messages on throws
{
	auto result = new Socket();

	result->mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

#if defined(PLATFORM_WINDOWS)
	if (result->mSocket == INVALID_SOCKET)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (result->mSocket == -1)
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
	if (bind(result->mSocket, (SOCKADDR*)&adr, adr_size) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (bind(result->mSocket, (sockaddr*)&adr, adr_size) == -1)
#endif
	{
	//	throw std::runtime_error("error while binding socket");
	}

#if defined(PLATFORM_WINDOWS)
	if (getsockname(result->mSocket, (SOCKADDR*)&adr, &adr_size) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (getsockname(result->mSocket, (sockaddr*)&adr, &adr_size) == -1)
#endif
	{
	//	throw std::runtime_error("error while getting sock port");
	}

	result->mPort = ntohs(adr.sin_port);
		
	u_long tr = 1;

#if defined(PLATFORM_WINDOWS)
	if (ioctlsocket(result->mSocket, FIONBIO, &tr) == SOCKET_ERROR)
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	if (fcntl(result->mSocket, F_SETFL, fcntl(result->mSocket, F_GETFL) | O_NONBLOCK) == -1)
#endif
	{
	//	throw std::runtime_error("cannot set socket blocking state");
	}

	mSockets.push_back(result);
		
	return result;
}

void System::destroySocket(Socket* socket)
{
#if defined(PLATFORM_WINDOWS)
	closesocket(socket->mSocket);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	close(socket.mSocket);
#endif
	mSockets.remove(socket);
	delete socket;
}