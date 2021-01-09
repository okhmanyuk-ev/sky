#pragma once

#include <core/engine.h>
#include <network/system.h>
#include <map>

namespace Shared
{
	enum class ClientMessage : uint32_t // Client -> Server
	{
		Connect = 1
	};

	enum class ServerMessage : uint32_t // Server -> Client
	{
		ConnectAccepted = 1
	};

	class Networking
	{
	public:
		using ReadCallback = std::function<void(Network::Packet&)>;

	public:
		Networking(uint16_t port = 0);

	protected:
		void addMessage(uint32_t msg, ReadCallback callback);
		void sendPacket(const Network::Packet& packet);

	private:
		void readPacket(Network::Packet& packet);

	private:
		Network::Socket mSocket;
		std::map<uint32_t, ReadCallback> mMessages;
	};

	class Server : public Networking
	{
	public:
		Server(uint16_t port);
	};

	class Client : public Networking
	{
	public:
		Client(const Network::Address& server_address);
		
	public:
		bool isConnected() const { return mConnected; }

	private:
		Network::Address mServerAddress;
		bool mConnected = false;
	};
}
