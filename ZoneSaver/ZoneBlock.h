#pragma once

#include "../CWSDK/cwsdk.h"

#pragma pack(push, 1)
namespace ZoneSaver {
	class ZoneBlock {
	public:
		u8 x, y;
		i32 z;
		u8 r, g, b, type;
		ZoneBlock();
		ZoneBlock(u8 x, u8 y, i32 z, u8 r, u8 g, u8 b, u8 type);
	};
}
#pragma pack(pop)