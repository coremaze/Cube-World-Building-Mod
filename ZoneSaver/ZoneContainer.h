#pragma once

#include "../CWSDK/cwsdk.h"

namespace ZoneSaver {
	class ZoneContainer {
	public:
		std::vector<ZoneBlock*> blocks;
		i32 zone_x, zone_y;

		ZoneContainer(i32 zonex, i32 zoney);

		void SetZoneBlock(u8 x, u8 y, u8 z, u8 r, u8 g, u8 b, u8 type);

		void OutputFile();
	};
}

