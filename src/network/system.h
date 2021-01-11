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
#include <common/timer.h>

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

		static const size_t inline BufferSize = 1024 * 64; // 64kb
		char mBuffer[BufferSize];

	public:
		auto getIncomingPacketsCount() const { return mIncomingPacketsCount; }
		auto getOutgoingPacketsCount() const { return mOutgoingPacketsCount; }
		auto getPacketsCount() const { return getIncomingPacketsCount() + getOutgoingPacketsCount(); }

		auto getIncomingPacketsPerSecond() const { return mIncomingPacketsPerSecond; }
		auto getOutgoingPacketsPerSecond() const { return mOutgoingPacketsPerSecond; }
		auto getPacketsPerSecond() const { return getIncomingPacketsPerSecond() + getOutgoingPacketsPerSecond(); }

		auto getIncomingBytesCount() const { return mIncomingBytesCount; }
		auto getOutgoingBytesCount() const { return mOutgoingBytesCount; }
		auto getBytesCount() const { return getIncomingBytesCount() + getOutgoingBytesCount(); }

		auto getIncomingBytesPerSecond() const { return mIncomingBytesPerSecond; }
		auto getOutgoingBytesPerSecond() const { return mOutgoingBytesPerSecond; }
		auto getBytesPerSecond() const { return getIncomingBytesPerSecond() + getOutgoingBytesPerSecond(); }

	private:
		uint64_t mIncomingPacketsCount = 0;
		uint64_t mOutgoingPacketsCount = 0;

		uint64_t mIncomingPacketsPerSecond = 0;
		uint64_t mOutgoingPacketsPerSecond = 0;
		uint64_t mPrevIncomingPacketsPerSecond = 0;
		uint64_t mPrevOutgoingPacketsPerSecond = 0;

		uint64_t mIncomingBytesCount = 0;
		uint64_t mOutgoingBytesCount = 0;

		uint64_t mIncomingBytesPerSecond = 0;
		uint64_t mOutgoingBytesPerSecond = 0;
		uint64_t mPrevIncomingBytesPerSecond = 0;
		uint64_t mPrevOutgoingBytesPerSecond = 0;

		Common::Timer mPacketsPerSecondTimer;
	};

	class Socket
	{
	public:
		Socket(uint16_t port = 0);
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