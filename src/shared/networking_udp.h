#pragma once

#include <network/system.h>
#include <map>
#include <common/hash.h>

namespace Shared::NetworkingUDP
{
	class Networking
	{
	public:
		static const int inline ProtocolVersion = 3;

		static int inline NetLogs = 0;
		static int inline NetReconnectDelay = 2; // sec
		static int inline NetTimeout = 15; // sec
		static int inline NetTransmitDurationMin = 100; // msec
		static int inline NetTransmitDurationMax = 2000; // msec

		static void Log(const std::string& text, int level);

	public:
		enum class Message : uint32_t // Client <-> Server (connectionless)
		{
			Regular = 0,
			Connect = 1,
		};

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
		Network::UdpSocket mSocket;
		std::map<uint32_t, ReadCallback> mMessages;
	};

	class Channel : public Common::FrameSystem::Frameable
	{
	public:
		using ReadCallback = std::function<void(Common::BitBuffer&)>;
		using WriteCallback = std::function<void(Common::BitBuffer&)>;
		using SendCallback = std::function<void(Common::BitBuffer&)>;
		using DisconnectCallback = std::function<void(const std::string& reason)>;

	private:
		void frame() override;
		void transmit();
		void awake();
		bool wantSendReliable() const;
		void readReliableDataFromPacket(Common::BitBuffer& buf);

	public:
		void read(Common::BitBuffer& buf);
		void sendReliable(const std::string& msg, Common::BitBuffer& buf);
		void addMessageReader(const std::string& msg, ReadCallback callback);
		void disconnect(const std::string& reason);

	private:
		float mTransmitDuration = 0.0f; // min(0.0)..max(1.0)
		Clock::TimePoint mAwakeTime = Clock::Now();

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		void setDisconnectCallback(DisconnectCallback value) { mDisconnectCallback = value; }

	public:
		auto getIncomingSequence() const { return mIncomingSequence; }
		auto getOutgoingSequence() const { return mOutgoingSequence; }
		
		auto getIncomingReliableIndex() const { return mIncomingReliableIndex; }
		auto getOutgoingReliableIndex() const { return mOutgoingReliableIndex; }

		auto getOutgoingReliableQueueSize() const { return mReliableMessages.size(); }

	private:
		SendCallback mSendCallback = nullptr;
		DisconnectCallback mDisconnectCallback = nullptr;

		Clock::TimePoint mTransmitTime = Clock::Now();
		Clock::TimePoint mIncomingTime = Clock::Now();

		uint32_t mOutgoingSequence = 0;
		uint32_t mIncomingSequence = 0;
		uint32_t mIncomingAcknowledgement = 0;

		bool mOutgoingReliableSequence = false;
		bool mIncomingReliableSequence = false;
		bool mIncomingReliableAcknowledgement = false;

		uint32_t mIncomingReliableIndex = 0;
		uint32_t mOutgoingReliableIndex = 1;

		uint32_t mReliableSentSequence = 0;

		std::list<std::pair<std::string, std::shared_ptr<Common::BitBuffer>>> mReliableMessages;
		std::map<std::string, ReadCallback> mMessageReaders;
	};

	class Server : public Networking
	{
	public:
		Server(uint16_t port);

	protected:
		virtual std::shared_ptr<Channel> createChannel() = 0;

	public:
		size_t getClientsCount() const { return mChannels.size(); }

		auto getChannels() const { return mChannels; }

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

	class Client : public Networking,
		public Common::FrameSystem::Frameable
	{
	public:
		Client(const Network::Address& server_address);

	protected:
		virtual std::shared_ptr<Channel> createChannel() = 0;

	public:
		void frame() override;

	private:
		void connect();

	public:
		bool isConnected() const { return mChannel != nullptr; }
		auto getChannel() const { return mChannel; }

	private:
		Network::Address mServerAddress;
		std::shared_ptr<Channel> mChannel = nullptr;
		Clock::TimePoint mConnectTime = Clock::Now();
	};

	class SimpleChannel : public Channel
	{
	public:
		using EventCallback = std::function<void(std::map<std::string, std::string>)>;
		
	public:
		SimpleChannel();

	public:
		void sendEvent(const std::string& name, const std::map<std::string, std::string>& params = {});

	private:
		void onEventMessage(Common::BitBuffer& buf);

	public:
		void addEventCallback(const std::string& name, EventCallback callback);

	private:
		std::map<std::string, EventCallback> mEvents;
	};
}