#pragma once
#include "cwmods/cwsdk.h"
class ChunkUpdate {
public:
	cube::Block block;
	LongVector3 position;
	ChunkUpdate(cube::Block block, LongVector3 position);
};
