#pragma once

#include <functional>
#include <unordered_set>
#include <core/engine.h>
#include <common/bitbuffer.h>
#include <platform/defines.h>
#include <common/frame_system.h>
#include <common/timer.h>

#define NETWORK ENGINE->getSystem<Network::System>()

namespace Network
{
	struct Address
	{
	public:
		Address();
		Address(const std::string& adr);

	public:
		std::string toString() const;

	public:
		union
		{
			uint8_t b[4];
			uint32_t l;
		} ip;

		uint16_t port;

	public:
		inline bool operator==(const Address& a) const { return ip.l == a.ip.l && port == a.port; }
		inline bool operator!=(const Address& a) const { return !(*this == a); }
	};

	struct Packet
	{
		Address adr;
		sky::BitBuffer buf;
	};

	class System : public Common::FrameSystem::Frameable
	{
		friend Address;
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
		struct Impl;
		std::unique_ptr<Impl> mImpl;

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
