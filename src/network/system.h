#pragma once

#include <Core/engine.h>

#include "packet.h"
#include <functional>

#include <Platform/defines.h>

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <winsock.h>
#endif

#include <Common/frame_system.h>

namespace Network
{
	class Socket
	{
		friend class System;

	public:
		typedef std::function<void(Packet&)> ReadCallback;

	public:
		void sendPacket(const Packet& packet);

	public:
		auto& setReadCallback(ReadCallback value) { mReadCallback = value; return *this; }

		auto getPort() const { return mPort; }

	private:
		ReadCallback mReadCallback = nullptr;
		uint16_t mPort = 0;

#if defined(PLATFORM_WINDOWS)
		SOCKET mSocket;
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
		int mSocket;
#endif
	};

	class System :  public Common::FrameSystem::Frameable
	{
	public:
		System();
		~System();

	public:
		void frame() override;

	public:
		Socket& createSocket(uint16_t port = 0);
		void destroySocket(Socket& socket);
	
	private:
		std::list<Socket*> mSockets;

		char mBuffer[8192];
	};
}