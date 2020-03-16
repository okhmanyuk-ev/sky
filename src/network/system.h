#pragma once

#include <core/engine.h>

#include "packet.h"
#include <functional>

#include <platform/defines.h>

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <winsock.h>
#endif

#include <common/frame_system.h>
#include <unordered_set>

#define NETWORK ENGINE->getSystem<Network::System>()

namespace Network
{
	class System : public Common::FrameSystem::Frameable
	{
	public:
		System();
		~System();

	private:
		void frame() override;
	
	public:
		using SocketHandle = void*;
		using ReadCallback = std::function<void(Packet&)>;

	public:
		SocketHandle createSocket(uint16_t port = 0);
		void destroySocket(SocketHandle handle);

		void sendPacket(SocketHandle handle, const Packet& packet);

		void setReadCallback(SocketHandle handle, ReadCallback value);
		uint64_t getPort(SocketHandle handle) const;

	private:
		struct SocketData
		{
			uint64_t port;
			ReadCallback readCallback = nullptr;
#if defined(PLATFORM_WINDOWS)
			SOCKET socket;
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
			int socket;
#endif
		};

	private:
		std::unordered_set<SocketData*> mSockets;
		char mBuffer[8192];
	};

	class Socket
	{
	public:
		Socket(uint64_t port = 0);
		~Socket();

	public:
		void sendPacket(const Packet& packet);

	public:
		void setReadCallback(System::ReadCallback value);
		auto getPort() const;

	private:
		System::SocketHandle mHandle = 0;
	};

}