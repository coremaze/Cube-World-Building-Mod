#include "ZoneBlock.h"

ZoneSaver::ZoneBlock::ZoneBlock() {
	this->x = 0;
	this->y = 0;
	this->z = 0;
	this->r = 0;
	this->g = 0;
	this->b = 0;
	this->type = 0;
}

ZoneSaver::ZoneBlock::ZoneBlock(u8 x, u8 y, i32 z, u8 r, u8 g, u8 b, u8 type) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->r = r;
	this->g = g;
	this->b = b;
	this->type = type;
}
