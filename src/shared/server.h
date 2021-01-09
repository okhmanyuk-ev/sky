#pragma once

#include <core/engine.h>
#include <network/system.h>

namespace Shared
{
	class Server
	{
	public:
		Server(uint16_t port);

	private:
		Network::Socket mSocket;
	};

	class Client
	{
	public:
		Client(const Network::Address& server_address);
		
	public:
		bool isConnected() const { return mConnected; }

	private:
		bool mConnected = false;
	};
}
