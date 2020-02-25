#include "ZoneSaver.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <algorithm>

ZoneSaver::ZoneContainer::ZoneContainer(i32 zonex, i32 zoney) {
	this->zone_x = zonex;
	this->zone_y = zoney;
}

void ZoneSaver::ZoneContainer::SetZoneBlock(u8 x, u8 y, u8 z, u8 r, u8 g, u8 b, u8 type) {
	//Update existing block if it exists.
	for (ZoneBlock* block : this->blocks) {
		if (block->x == x && block->y == y && block->z == z) {
			block->r = r;
			block->g = g;
			block->b = b;
			block->type = type;
			return;
		}
	}

	//Add a new block if does not exist
	ZoneBlock* block = new ZoneBlock(x, y, z, r, g, b, type);
	this->blocks.push_back(block);
}

void ZoneSaver::ZoneContainer::OutputFile() {
	char fileName[256] = { 0 };
	const char* folderName = GetFolderName();

	CreateDirectory(folderName, NULL);

	//File for this Zone
	sprintf(fileName, "%s\\%d.%d.cwb", folderName, this->zone_x, this->zone_y);
	std::ofstream file;
	file.open(fileName, std::ios::out | std::ios::binary);

	//Write each block to file
	for (ZoneBlock* block : this->blocks) {
		file.write((char*)block, sizeof(*block));
	}
	file.close();
}