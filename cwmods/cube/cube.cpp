#include "cube.h"

cube::Game* cube::GetGame() {
	return *(cube::Game**)(CWOffset(0x551A80));
}

ID3D11Device* cube::GetID3D11Device() {
	return *(ID3D11Device**)(CWOffset(0x551A90));
}

ID3D11DeviceContext* cube::GetID3D11DeviceContext() {
	return *(ID3D11DeviceContext**)(CWOffset(0x551A98));
}