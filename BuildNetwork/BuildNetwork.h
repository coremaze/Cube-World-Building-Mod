#pragma once

#include "../CWSDK/cwsdk.h"
#include "BuildNetworkPacket.h"
#include "SingleBlockUpdatePacket.h"
#include "PingPacket.h"
#include "ZoneUpdatePacket.h"
#include "ZoneLoadPacket.h"

class BuildingMod;
class BuildNetwork {
	BuildingMod* mod;
	int CSChannel;
	int SCChannel;
	u32 lastPing;
	u32 GetPingTime();
	void UpdatePingTime();
	void DoPings();
public:
	BuildNetwork(BuildingMod* mod);
	void SendSingleBlockUpdate(CSteamID steamID, LongVector3& position, cube::Block& block, bool toClient);
	void SendPing(CSteamID steamID, bool toClient);
	void SendZoneLoad(CSteamID steamID, IntVector2 position);
	void Process();
	void ProcessSC();
	void ProcessCS();

};

