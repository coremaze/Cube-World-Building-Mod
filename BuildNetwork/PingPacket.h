#pragma once
#include "BuildNetworkPacket.h"
#include "../cwmods/cwsdk.h"

#pragma pack(push, 1)
class PingPacket : public BuildNetworkPacket {
public:
	void Send(CSteamID steamID, i32 channel);
	PingPacket();
};
#pragma pack(pop)