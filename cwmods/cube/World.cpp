#include "World.h"
#include "Zone.h"
#include "constants.h"
#include "../common/LongVector3.h"
#include "../common/IntVector3.h"
#include "../cwmods.h"

cube::Zone* cube::World::GetZone(IntVector2 position) {
	try {
		return this->zones.at(position);
	}
	catch (const std::out_of_range & oor) {
		return nullptr;
	}
	
}
cube::Zone* cube::World::GetZone(int x, int y) {
	IntVector2 position(x, y);
	return this->GetZone(position);
}
void cube::World::SetBlock(LongVector3 block_pos, Block block) {
	IntVector2 zonePos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
	cube::Zone* zone = this->GetZone(zonePos);
	if (!zone) return;
	IntVector3 zoneBlockPos(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);
	zone->SetBlock(zoneBlockPos, block);
}
cube::Block* cube::World::GetBlock(LongVector3 block_pos) {
	IntVector2 zonePos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
	cube::Zone* zone = this->GetZone(zonePos);
	if (!zone) return nullptr;
	IntVector3 zoneBlockPos(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);

	return zone->GetBlock(zoneBlockPos);
}

cube::Block cube::World::GetBlockInterpolated(LongVector3 block_pos) {
	IntVector2 zonePos = cube::Zone::ZoneCoordsFromBlocks(block_pos.x, block_pos.y);
	cube::Zone* zone = this->GetZone(zonePos);

	if (!zone) {
		cube::Block block;
		block.red = 255;
		block.green = 255;
		block.blue = 255;
		block.field_3 = 0;
		block.type = 0;
		block.breakable = 0;
		return block;
	}
	IntVector3 zoneBlockPos(pymod(block_pos.x, cube::BLOCKS_PER_ZONE), pymod(block_pos.y, cube::BLOCKS_PER_ZONE), block_pos.z);

	return zone->GetBlockInterpolated(zoneBlockPos);
}

cube::Block* cube::World::GetBlock(uint64_t block_x, uint64_t block_y, uint64_t block_z) {
	LongVector3 position(block_x, block_y, block_z);
	return this->GetBlock(position);
}

cube::Block cube::World::GetBlockInterpolated(uint64_t block_x, uint64_t block_y, uint64_t block_z) {
	LongVector3 position(block_x, block_y, block_z);
	return this->GetBlockInterpolated(position);
}

void cube::World::SetTime(float ms) {
	this->state.time = ms;
}
void cube::World::SetTime(int ms) {
	this->state.time = (float)ms;
}
void cube::World::SetTime(int hour, int minute) {
	this->state.time = (float)((hour * 60 * 60 * 1000) + (minute * 60 * 1000));
}
float cube::World::GetTime() {
	return this->state.time;
}