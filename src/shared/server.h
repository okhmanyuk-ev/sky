#pragma once

#include <core/engine.h>
#include <network/system.h>

#define SERVER ENGINE->getSystem<Shared::Server>()
#define CLIENT ENGINE->getSystem<Shared::Client>()

namespace Shared
{
	class Server
	{
		//
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
