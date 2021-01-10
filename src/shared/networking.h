#pragma once

#include <core/engine.h>
#include <network/system.h>
#include <map>
#include <common/hash.h>

namespace Shared::Networking
{
	class Networking
	{
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
		Network::Socket mSocket;
		std::map<uint32_t, ReadCallback> mMessages;
	};

	class Channel : public Common::FrameSystem::Frameable
	{
	public:
		using ReadCallback = std::function<void(Common::BitBuffer&)>;
		using WriteCallback = std::function<void(Common::BitBuffer&)>;
		using SendCallback = std::function<void(Common::BitBuffer&)>;
		using DisconnectCallback = std::function<void(const std::string& reason)>;

	public:
		Channel();

	private:
		void frame() override;
		void transmit();
		void awake();
		bool awaitingReliableAcknowledgement() const;
		
	public:
		void read(Common::BitBuffer& buf);
		void sendReliable(uint32_t msg, Common::BitBuffer& buf); // TODO: rename to writeReliableMessage
		void addMessageReader(uint32_t msg, ReadCallback callback);
		void addMessageWriter(uint32_t msg, WriteCallback callback);
		void disconnect(const std::string& reason);

	private:
		Clock::Duration mTimeoutDuration = Clock::FromSeconds(30);
		Clock::Duration mTransmitDurationMin = Clock::FromMilliseconds(10);
		Clock::Duration mTransmitDurationMax = Clock::FromMilliseconds(2000);
		float mTransmitDuration = 0.0f; // min(0.0)..max(1.0)
		Clock::TimePoint mAwakeTime = Clock::Now();

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		void setDisconnectCallback(DisconnectCallback value) { mDisconnectCallback = value; }

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

		uint32_t mReliableSequence = 0;

		std::list<std::pair<uint32_t, std::shared_ptr<Common::BitBuffer>>> mReliableMessages;
		std::map<uint32_t, ReadCallback> mMessageReaders;
		std::map<uint32_t, WriteCallback> mMessageWriters; // regular
	};

	class Server : public Networking
	{
	public:
		class Channel;

	public:
		Server(uint16_t port);

	public:
		virtual std::shared_ptr<Server::Channel> createChannel() = 0;
		
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

	class Server::Channel : public Shared::Networking::Channel
	{
	public:
		enum class Message : uint32_t // Server -> Client
		{
			Event = 0
		};

	public:
		Channel();

	public:
		void sendEvent(const std::string& name, const std::map<std::string, std::string>& params);

	protected:
		virtual void onEvent(const std::string& name, const std::map<std::string, std::string>& params) = 0;
	};

	class Client : public Networking,
		public Common::FrameSystem::Frameable
	{
	public:
		enum class Message : uint32_t // Client -> Server
		{
			Event = 0
		};

	public:
		Client(const Network::Address& server_address);

	public:
		void frame() override;

	private:
		void connect();

	public:
		void sendEvent(const std::string& name, const std::map<std::string, std::string>& params);

	protected:
		virtual void onEvent(const std::string& name, const std::map<std::string, std::string>& params) = 0;

	public:
		bool isConnected() const { return mChannel != nullptr; }

	private:
		Network::Address mServerAddress;
		std::shared_ptr<Channel> mChannel = nullptr;
		Clock::TimePoint mConnectTime = Clock::Now();
		Clock::Duration mReconnectDelay = Clock::FromSeconds(3);
	};
}
