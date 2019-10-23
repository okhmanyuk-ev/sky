#include "address.h"

namespace Network
{
	Address::Address()
	{
		//
	}

	Address::Address(std::string adr)
	{
		auto p = adr.find(":");
		
		if (p != std::string::npos)
		{
			port = stoi(adr.substr(p + 1));
			adr = adr.substr(0, p);
		}
		else
		{
			port = 0;
		}

		for (int i = 0; i < 3; i++)
		{
			auto pp = adr.find(".");
			ip.b[i] = stoi(adr.substr(0, pp));
			adr = adr.substr(pp + 1);
		}
		
		ip.b[3] = stoi(adr);
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
}