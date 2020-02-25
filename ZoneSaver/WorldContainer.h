#pragma once
#include "../cwmods/cwsdk.h"
#include "ZoneSaver.h"
namespace ZoneSaver {
	class WorldContainer {
	public:
		std::vector<ZoneContainer*> zones;

		ZoneContainer* SetBlock(LongVector3& position, unsigned char r, unsigned char g, unsigned char b, unsigned char type);
		void OutputFiles();
		std::vector<ZoneBlock*> LoadZoneBlocks(i32 zone_x, i32 zone_y);
		bool DeleteZoneContainer(unsigned int zone_x, unsigned int zone_y);
	};
}