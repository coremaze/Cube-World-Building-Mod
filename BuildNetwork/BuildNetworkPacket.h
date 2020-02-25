#pragma once

#include "../cwmods/cwsdk.h"

#pragma pack(push, 1)
class BuildNetworkPacket {
public:
	enum PacketType : u8 {
		None = 0,
		Ping,
		ZoneLoad,
		SingleBlockUpdate,
		ZoneUpdate,
	};
	PacketType id;
};
#pragma pack(pop)