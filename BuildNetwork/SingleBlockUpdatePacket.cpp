#include "SingleBlockUpdatePacket.h"
#include "../CWSDK/cwsdk.h"

void SingleBlockUpdatePacket::Send(CSteamID steamID, i32 channel) {
	cube::SteamNetworking()->SendP2PPacket(steamID, (const void*)this, sizeof(*this), k_EP2PSendReliable, channel);
}

SingleBlockUpdatePacket::SingleBlockUpdatePacket(LongVector3& position, cube::Block& block) {
	this->id = SingleBlockUpdate;
	this->position = position;
	this->block = block;
}