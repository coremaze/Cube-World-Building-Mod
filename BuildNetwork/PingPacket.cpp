#include "PingPacket.h"
#include "../cwmods/cwsdk.h"

void PingPacket::Send(CSteamID steamID, i32 channel) {
	cube::SteamNetworking()->SendP2PPacket(steamID, (const void*)this, sizeof(*this), k_EP2PSendReliable, channel);
}

PingPacket::PingPacket() {
	this->id = Ping;
}