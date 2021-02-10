#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <core/engine.h>

#include "packet.h"
#include <functional>

#include <platform/defines.h>
#include <asio.hpp>

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
		void onFrame() override;
	
	public:
		using UdpSocketHandle = void*;
		using ReadCallback = std::function<void(Packet&)>;

	public:
		UdpSocketHandle createUdpSocket(uint16_t port = 0);
		void destroyUdpSocket(UdpSocketHandle handle);
		void sendUdpPacket(UdpSocketHandle handle, const Packet& packet);
		void setUdpReadCallback(UdpSocketHandle handle, ReadCallback value);
		uint16_t getUdpSocketPort(UdpSocketHandle handle) const;

	private:
		struct UdpSocketData
		{
			UdpSocketData(asio::io_service& service, const asio::ip::udp::endpoint& endpoint) : socket(service, endpoint) {}
			ReadCallback readCallback = nullptr;
			asio::ip::udp::socket socket;
		};

	private:
		std::unordered_set<UdpSocketData*> mUdpSockets;
		asio::io_service mService;

	public:
		auto& getIoService() { return mService; }

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

	class UdpSocket
	{
	public:
		UdpSocket(uint16_t port = 0);
		~UdpSocket();

	public:
		void sendPacket(const Packet& packet);

	public:
		void setReadCallback(System::ReadCallback value);
		uint16_t getPort() const;

	private:
		System::UdpSocketHandle mHandle = 0;
	};
}