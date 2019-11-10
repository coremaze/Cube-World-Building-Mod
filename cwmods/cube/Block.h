#ifndef CUBE_BLOCK_H
#define CUBE_BLOCK_H
#include "../common/types.h"
#include "../IDA/types.h"

namespace cube {
class Block {
    public:
		enum Type {
			Air     = 0b00000,
			Colored = 0b00001,
			Water   = 0b00010,
			Lava    = 0b10000
		};

		u8 red;
		u8 green;
		u8 blue;
		u8 field_3;
		u8 type;
		u8 breakable;
    };
}

static_assert(sizeof(cube::Block) == 0x6, "cube::Block is not the correct size.");

#endif // CUBE_BLOCK_H
