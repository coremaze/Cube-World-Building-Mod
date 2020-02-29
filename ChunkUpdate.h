#pragma once
#include "CWSDK/cwsdk.h"
class ChunkUpdate {
public:
	cube::Block block;
	LongVector3 position;
	ChunkUpdate(cube::Block block, LongVector3 position);
};
