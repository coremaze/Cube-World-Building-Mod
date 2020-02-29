#include "ZoneLoadPacket.h"
#include "../CWSDK/cwsdk.h"

void ZoneLoadPacket::Send(CSteamID steamID, i32 channel) {
	cube::SteamNetworking()->SendP2PPacket(steamID, (const void*)this, sizeof(*this), k_EP2PSendReliable, channel);
}

ZoneLoadPacket::ZoneLoadPacket(IntVector2& position) {
	this->id = ZoneLoad;
	this->position = position;
}