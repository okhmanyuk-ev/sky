#pragma once

#include <cstdint>
#include <string>

namespace Network
{
	struct Address
	{
	public:
		Address();
		Address(std::string adr);

	public:	
		std::string toString() const;
	
	public:
		union
		{
			uint8_t b[4];
			uint32_t l;
		} ip;

		uint16_t port;

	public:
		inline bool operator==(const Address& a) const { return ip.l == a.ip.l && port == a.port; }
		inline bool operator!=(const Address& a) const { return !(*this == a); }
	};
}