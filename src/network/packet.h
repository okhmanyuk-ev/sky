#pragma once

#include <common/bitbuffer.h>
#include "address.h"

namespace Network
{
	struct Packet
	{
		Address adr;
		Common::BitBuffer buf;
	};
}
