#pragma once

#include <core/engine.h>
#include <network/system.h>
#include <map>
#include <common/hash.h>

namespace Shared
{
	enum class Message : uint32_t // Client <-> Server
	{
		Regular = 0,
		Connect = 1,
	};

	class Networking
	{
	public:
		using ReadCallback = std::function<void(Network::Packet&)>;

	public:
		Networking(uint16_t port = 0);

	protected:
		void addMessage(uint32_t msg, ReadCallback callback);
		void sendMessage(uint32_t msg, const Network::Address& adr, const Common::BitBuffer& buf = {});

	private:
		void readPacket(Network::Packet& packet);

	private:
		Network::Socket mSocket;
		std::map<uint32_t, ReadCallback> mMessages;
	};

	class Channel : public Common::FrameSystem::Frameable
	{
	public:
		using SendCallback = std::function<void(Common::BitBuffer&)>;
		using TimeoutCallback = std::function<void()>;

	public:
		Channel();

	private:
		void frame() override;
		void transmit();

	public:
		void read(Common::BitBuffer& buf);
		void sendReliable(Common::BitBuffer& buf);

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		void setTimeoutCallback(TimeoutCallback value) { mTimeoutCallback = value; }

	private:
		SendCallback mSendCallback = nullptr;
		TimeoutCallback mTimeoutCallback = nullptr;

		Clock::TimePoint mTransmitTime = Clock::Now();
		Clock::TimePoint mIncomingTime = Clock::Now();

		uint32_t mOutgoingSequence = 0;
		uint32_t mIncomingSequence = 0;
		uint32_t mIncomingAcknowledgement = 0;

		bool mOutgoingReliableSequence = false;
		bool mIncomingReliableSequence = false;
		bool mIncomingReliableAcknowledgement = false;

		uint32_t mReliableSequence = 0;

		std::list<std::shared_ptr<Common::BitBuffer>> mReliableMessages;
	};

	class Server : public Networking
	{
	public:
		Server(uint16_t port);

	public:
		size_t getClientsCount() const { return mChannels.size(); }

	private:
		struct AddressHasher
		{
			size_t operator()(const Network::Address& adr) const
			{
				size_t seed = 0;
				Common::Hash::combine(seed, adr.ip.l);
				Common::Hash::combine(seed, adr.port);
				return seed;
			}
		};

		std::unordered_map<Network::Address, std::shared_ptr<Channel>, AddressHasher> mChannels;
	};

	class Client : public Networking
	{
	public:
		Client(const Network::Address& server_address);

	public:
		bool isConnected() const { return mChannel != nullptr; }

	private:
		Network::Address mServerAddress;
		std::shared_ptr<Channel> mChannel = nullptr;
	};
}
