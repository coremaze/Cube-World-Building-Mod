#pragma once
#include "BuildNetworkPacket.h"
#include "../cwmods/cwsdk.h"

#pragma pack(push, 1)
class SingleBlockUpdatePacket : public BuildNetworkPacket {
public:
	LongVector3 position;
	cube::Block block;

	void Send(CSteamID steamID, i32 channel);
	SingleBlockUpdatePacket(LongVector3& position, cube::Block& block);

};
#pragma pack(pop)