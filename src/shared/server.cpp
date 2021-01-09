#include "server.h"
#include <console/device.h>

using namespace Shared;

// server

Server::Server(uint16_t port) : mSocket(port)
{
	//
}

// client

Client::Client(const Network::Address& server_address)
{
	LOG("connecting to " + server_address.toString());
}
