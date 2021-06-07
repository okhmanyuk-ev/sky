#include "address.h"
#include <asio.hpp>
#include "system.h"

using namespace Network;

Address::Address()
{
	//
}

Address::Address(const std::string& adr)
{
	auto ip_s = adr;
	
	auto p = adr.find(":");

	if (p != std::string::npos)
	{
		port = stoi(adr.substr(p + 1));
		ip_s = adr.substr(0, p);
	}
	else
	{
		port = 0;
	}

	try
	{
		auto bytes = asio::ip::address_v4::from_string(ip_s).to_bytes();

		ip.b[0] = bytes[0];
		ip.b[1] = bytes[1];
		ip.b[2] = bytes[2];
		ip.b[3] = bytes[3];
	}
	catch (...)
	{
		auto resolver = asio::ip::udp::resolver(NETWORK->getIoService());
		auto query = asio::ip::udp::resolver::query(ip_s, "");
		auto results = resolver.resolve(query);
		for (auto entry : results)
		{
			auto bytes = entry.endpoint().address().to_v4().to_bytes();
			ip.b[0] = bytes[0];
			ip.b[1] = bytes[1];
			ip.b[2] = bytes[2];
			ip.b[3] = bytes[3];
			break;
		}
	}
}

std::string Address::toString() const
{
	return 
		std::to_string(ip.b[0]) + "." + 
		std::to_string(ip.b[1]) + "." +
		std::to_string(ip.b[2]) + "." +
		std::to_string(ip.b[3]) + ":" +
		std::to_string(port);
}
