#ifndef CUBE_H
#define CUBE_H
#include "../cwmods.h"

namespace cube {
	class Game;
	cube::Game* GetGame();
	ID3D11Device* GetID3D11Device();
	ID3D11DeviceContext* GetID3D11DeviceContext();
}

#endif // CUBE_H
