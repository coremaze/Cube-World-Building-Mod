#include "ZoneSaver.h"

ZoneSaver::ZoneContainer* ZoneSaver::WorldContainer::SetBlock(LongVector3& position, u8 r, u8 g, u8 b, u8 type) {

	u8 x = pymod(position.x, cube::BLOCKS_PER_ZONE);
	u8 y = pymod(position.y, cube::BLOCKS_PER_ZONE);
	i32 z = position.z;

	IntVector2 zonePos(pydiv(position.x, cube::BLOCKS_PER_ZONE), pydiv(position.y, cube::BLOCKS_PER_ZONE));

	//check to see if there is already a zone for this block
	for (ZoneContainer* zc : zones) {
		if ((zc->zone_x == zonePos.x) && (zc->zone_y == zonePos.y)) {
			zc->SetZoneBlock(x, y, z, r, g, b, type);
			return zc;
		}
	}

	//No zone created yet
	ZoneContainer* zc = new ZoneContainer(zonePos.x, zonePos.y);
	zc->SetZoneBlock(x, y, z, r, g, b, type);
	this->zones.push_back(zc);
	return zc;
}

void ZoneSaver::WorldContainer::OutputFiles() {
	for (ZoneContainer* zc : zones) {
		zc->OutputFile();
	}
}

std::vector<ZoneSaver::ZoneBlock*> ZoneSaver::WorldContainer::LoadZoneBlocks(i32 zone_x, i32 zone_y) {
	char* blocks;
	std::streampos fsize;
	std::vector<ZoneBlock*> emptyBlocks;

	//First, check to see if the zone is already in memory.
	for (ZoneContainer* zc : zones) {
		if ((zc->zone_x == zone_x) && (zc->zone_y == zone_y)) {
			return zc->blocks;
		}
	}

	//If not, look for the zone file, and try to make the zone from that.
	char fileName[256] = { 0 };
	const char* folderName = GetFolderName();

	//Ensure this directory is made
	CreateDirectory(folderName, NULL);

	//File for this Zone
	ZoneContainer* zc = nullptr;
	sprintf(fileName, "%s\\%d.%d.cwb", folderName, zone_x, zone_y);
	std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		//File exists, read it
		fsize = file.tellg();
		blocks = new char[fsize];
		file.seekg(0, std::ios::beg);
		file.read(blocks, fsize);
		file.close();

		//SetBlock for every block found in the save file.
		//This also causes a cube::Zone to be created if it is not.
		for (int i = 0; i < fsize; i += sizeof(ZoneBlock)) {
			ZoneBlock* zoneBlock = (ZoneBlock*)&blocks[i];
			LongVector3 position( zoneBlock->x + zone_x * cube::BLOCKS_PER_ZONE, 
				                  zoneBlock->y + zone_y * cube::BLOCKS_PER_ZONE,
								  zoneBlock->z);
			zc = this->SetBlock(position, zoneBlock->r, zoneBlock->g, zoneBlock->b, zoneBlock->type);
		}

		delete[] blocks;

		if (zc != nullptr) {
			return zc->blocks;
		}
		else {
			//This will only happen if a blocks file was somehow empty.
			return emptyBlocks;
		}



	}
	else {
		//The zone file does not exist. We don't need to make
		//one until the user places a block, so return an empty vector.
		return emptyBlocks;
	}
}

//Deletes the first ZoneContainer with the specified coordinate, returns whether it found one to delete.
bool ZoneSaver::WorldContainer::DeleteZoneContainer(unsigned int zone_x, unsigned int zone_y) {
	for (ZoneContainer* zone : this->zones) {
		if (zone->zone_x == zone_x && zone->zone_y == zone_y) {
			std::vector<ZoneContainer*>::iterator position = std::find(this->zones.begin(), this->zones.end(), zone);
			if (position != this->zones.end()) {
				this->zones.erase(position);
				delete zone;
				return true;
			}

		}
	}
	return false;
}