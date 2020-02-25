#pragma once
#include "BuildNetworkPacket.h"
#include "../cwmods/cwsdk.h"

#pragma pack(push, 1)
class ZoneLoadPacket : public BuildNetworkPacket {
public:
	IntVector2 position;

	void Send(CSteamID steamID, i32 channel);
	ZoneLoadPacket(IntVector2& position);
};
#pragma pack(pop)