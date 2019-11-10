#include "DButton.h"

DButton::DButton(int diKey) {
	oldState = 0;
	currentState = 0;
	this->diKey = diKey;
}

void DButton::Update(BYTE* diKeys) {
	oldState = currentState;
	currentState = diKeys[diKey];
}

bool DButton::Pressed() {
	return ((oldState & 0x80) == 0) && ((currentState & 0x80) != 0);
}