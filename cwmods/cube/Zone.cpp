#include "Zone.h"
#include "../cwmods.h"
#include "Block.h"

IntVector2 cube::Zone::ZoneCoordsFromDots(LongVector3 position) {
	int zone_x = pydiv(pydiv(position.x, cube::DOTS_PER_BLOCK), cube::BLOCKS_PER_ZONE);
	int zone_y = pydiv(pydiv(position.y, cube::DOTS_PER_BLOCK), cube::BLOCKS_PER_ZONE);
	return IntVector2(zone_x, zone_y);
}

IntVector2 cube::Zone::ZoneCoordsFromBlocks(int64_t x, int64_t y) {
	int zone_x = pydiv(x, cube::BLOCKS_PER_ZONE);
	int zone_y = pydiv(y, cube::BLOCKS_PER_ZONE);
	return IntVector2(zone_x, zone_y);
}
void cube::Zone::SetBlock(IntVector3 zone_position, cube::Block block) {
	int fieldIndex = zone_position.x * cube::BLOCKS_PER_ZONE + zone_position.y;
	cube::Field* field = &this->fields[fieldIndex];

	int block_index = zone_position.z - field->base_z;
	int end_z = field->base_z + field->blocks.size();

	if (zone_position.z >= end_z) { //Too high
		int block_index = zone_position.z - field->base_z;

		int old_count = field->blocks.size();
		field->blocks.resize(block_index + 1);
		int new_count = field->blocks.size();

		void* start_air = &field->blocks[old_count];
		int air_size = (new_count - old_count) * sizeof(cube::Block);
		memset(start_air, 0, air_size);

		field->blocks[block_index] = block;
	} 
	else if (zone_position.z < field->base_z) { //Too low
		int old_size = field->blocks.size() * sizeof(cube::Block);
		char* tmp_blocks = new char[old_size];
		memcpy(tmp_blocks, &field->blocks[0], old_size);

		int difference = field->base_z - zone_position.z;

		// Get interpolated blocks for those which are being created below base z
		cube::Block* filler_blocks = new cube::Block[difference];
		IntVector3 fill_location = zone_position;
		for (int i = 0; i < difference; i++) {
			filler_blocks[i] = this->GetBlockInterpolated(fill_location);
			fill_location.z++;
		}

		int required_blocks = field->blocks.size() + difference;

		field->blocks.resize(required_blocks);

		//memset(&field->blocks[0], 0, difference * sizeof(cube::Block)); //Air
		
		memcpy(&field->blocks[0], filler_blocks, difference * sizeof(cube::Block));
		memcpy(&field->blocks[difference], tmp_blocks, old_size);
		

		field->blocks[0] = block;

		field->base_z -= difference;

		delete[] tmp_blocks;
		delete[] filler_blocks;
	}
	else {
		int block_index = zone_position.z - field->base_z;
		field->blocks[block_index] = block;
	}

	this->chunk.needs_remesh = true;
}

cube::Block* cube::Zone::GetBlock(IntVector3 zone_position) {
	int fieldIndex = zone_position.x * cube::BLOCKS_PER_ZONE + zone_position.y;
	cube::Field* field = &this->fields[fieldIndex];
	if (zone_position.z < field->base_z) {
		return nullptr;
	}

	int block_index = zone_position.z - field->base_z;

	if (block_index >= field->blocks.size()) {
		return nullptr;
	}

	return &field->blocks[block_index];
}

cube::Block cube::Zone::GetBlockInterpolated(IntVector3 zone_position) {
	int fieldIndex = zone_position.x * cube::BLOCKS_PER_ZONE + zone_position.y;
	cube::Field* field = &this->fields[fieldIndex];

	if (zone_position.z < field->base_z) {
		// This does seem to reflect what the actual game uses for its equivalent function,
		// But what actually ends up being rendered can be different. (See cubeworld.exe+0xEA0D2)
		// cube::Field has some floats that affect what color sub-base-z blocks are rendered as.
		// I'm guessing it's biome related information.
		cube::Block block;
		block.red = 0xC8;
		block.green = 0xC8;
		block.blue = 0;
		block.field_3 = 0;
		block.type = 1;
		block.breakable = 0;
		return block;
	}

	int block_index = zone_position.z - field->base_z;

	if (block_index >= field->blocks.size()) {
		// Air
		cube::Block block;
		block.red = 255;
		block.green = 255;
		block.blue = 255;
		block.field_3 = 0;
		block.type = 0;
		block.breakable = 0;
		return block;
	}

	return field->blocks[block_index];
}