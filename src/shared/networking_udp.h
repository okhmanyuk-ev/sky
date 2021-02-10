#pragma once

#include <map>
#include <set>
#include <optional>
#include <network/system.h>
#include <common/hash.h>

namespace Shared::NetworkingUDP
{
	class Networking
	{
	public:
		static const int inline ProtocolVersion = 3;

		static int inline NetReconnectDelay = 2; // sec
		static int inline NetTimeout = 15; // sec
		static int inline NetTransmitDelayMin = 10; // msec
		static int inline NetTransmitDelayMax = 2000; // msec
		static int inline NetMaxPacketSize = 1100;
		static bool inline NetLogPackets = false;
		static bool inline NetLogLoss = false;
		static bool inline NetLogRel = false;

	public:
		enum class Message : uint32_t // Client <-> Server (connectionless)
		{
			Regular = 0,
			Connect = 1,
			Disconnect = 2,
			Redirect = 3
		};

	public:
		using ReadCallback = std::function<void(Network::Packet&)>;

	public:
		Networking(uint16_t port = 0);

	protected:
		void addMessage(uint32_t msg, ReadCallback callback);
		void sendMessage(uint32_t msg, const Network::Address& adr, const Common::BitBuffer& buf = {});
		void sendDisconnect(const Network::Address& address, const std::string& reason);
		void sendRedirect(const Network::Address& address, const std::string& redirect_address);

	private:
		void readPacket(Network::Packet& packet);

	public:
		const auto& getSocket() const { return mSocket; }

	private:
		Network::UdpSocket mSocket;
		std::map<uint32_t, ReadCallback> mMessages;
	};

	class Channel : public Common::FrameSystem::Frameable
	{
	public:
		using ReadCallback = std::function<void(Common::BitBuffer&)>;
		using SendCallback = std::function<void(Common::BitBuffer&)>;
		using DisconnectCallback = std::function<void(const std::string& reason)>;

	private:
		void frame() override;
		void transmit();
		void awake();
		void readReliableMessages();
		void resendReliableMessages(uint32_t ack);

	public:
		void read(Common::BitBuffer& buf);
		void sendReliable(const std::string& msg, Common::BitBuffer& buf);
		void addMessageReader(const std::string& msg, ReadCallback callback);
		void disconnect(const std::string& reason);

	public:
		void setSendCallback(SendCallback value) { mSendCallback = value; }
		void setDisconnectCallback(DisconnectCallback value) { mDisconnectCallback = value; }

	public:
		auto getHibernation() const { return mHibernation; }

		auto getIncomingSequence() const { return mIncomingSequence; }
		auto getOutgoingSequence() const { return mOutgoingSequence; }

		auto getIncomingReliableIndex() const { return mIncomingReliableIndex; }
		auto getOutgoingReliableIndex() const { return mOutgoingReliableIndex; }
		
	private:
		SendCallback mSendCallback = nullptr;
		DisconnectCallback mDisconnectCallback = nullptr;

		float mHibernation = 0.0f; // 0.0f..1.0f
		Clock::TimePoint mAwakeTime = Clock::Now();

		Clock::TimePoint mTransmitTime = Clock::Now();
		Clock::TimePoint mIncomingTime = Clock::Now();

		uint32_t mOutgoingSequence = 0;
		uint32_t mIncomingSequence = 0;

		uint32_t mOutgoingReliableIndex = 0;
		uint32_t mIncomingReliableIndex = 0;

		struct ReliableMessage
		{
			std::string name;
			std::shared_ptr<Common::BitBuffer> buf;
		};

		struct PendingReliableMessage
		{
			uint32_t sequence;
			ReliableMessage rel_msg;
		};

		std::map</*index*/uint32_t, ReliableMessage> mOutgoingReliableMessages;
		std::map</*index*/uint32_t, PendingReliableMessage> mPendingOutgoingReliableMessages;
		std::set</*index*/uint32_t> mReliableAcknowledgements;
		std::map</*index*/uint32_t, ReliableMessage> mIncomingReliableMessages;

		std::map<std::string, ReadCallback> mMessageReaders;

		std::optional</*reason*/std::string> mDisconnect;
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
		
	public:
		auto isShowEventLogs() const { return mShowEventLogs; }
		void setShowEventLogs(bool value) { mShowEventLogs = value; }

	private:
		bool mShowEventLogs = false;
	};
}