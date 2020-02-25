#pragma once
#include "BuildNetworkPacket.h"
#include "../cwmods/cwsdk.h"

#pragma pack(push, 1)
class ZoneUpdatePacket : public BuildNetworkPacket {
public:
	class Entry {
	public:
		u8 x, y;
		i32 z;
		cube::Block block;
		Entry(u8 x, u8 y, i32 z, cube::Block& block);
	};
	IntVector2 position;
	std::vector<Entry> entries;

	void Send(CSteamID steamID, i32 channel);
	ZoneUpdatePacket(IntVector2& position);
	ZoneUpdatePacket(u8* buffer, u32 size);

};
#pragma pack(pop)