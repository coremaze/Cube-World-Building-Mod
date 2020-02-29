#include "ZoneUpdatePacket.h"
#include "../CWSDK/cwsdk.h"

void ZoneUpdatePacket::Send(CSteamID steamID, i32 channel) {
	BytesIO bytesio;
	bytesio.Write<PacketType>(this->id);
	bytesio.Write<IntVector2>(this->position);
	bytesio.Write<u32>(entries.size());
	for (Entry& entry : entries) {
		bytesio.Write<Entry>(entry);
	}

	const void* buffer = (const void*)bytesio.Data();
	i32 size = bytesio.Size();
	cube::SteamNetworking()->SendP2PPacket(steamID, buffer, size, k_EP2PSendReliable, channel);
}

ZoneUpdatePacket::ZoneUpdatePacket(IntVector2& position) {
	this->id = ZoneUpdate;
	this->position = position;
}

ZoneUpdatePacket::ZoneUpdatePacket(u8* buffer, u32 size) {
	BytesIO bytesio(buffer, size);
	this->id = bytesio.Read<PacketType>();
	this->position = bytesio.Read<IntVector2>();
	u32 count = bytesio.Read<u32>();
	this->entries.clear();
	this->entries.reserve(count);
	for (int i = 0; i < count; i++) {
		this->entries.push_back(bytesio.Read<Entry>());
	}
}

ZoneUpdatePacket::Entry::Entry(u8 x, u8 y, i32 z, cube::Block& block) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->block = block;
}