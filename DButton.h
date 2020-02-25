#ifndef DBUTTON_H
#define DBUTTON_H
#include "cwmods/cwsdk.h"

class DButton {
private:
	BYTE oldState;
	BYTE currentState;

public:
	int diKey;
	DButton(int diKey);
	void Update(BYTE* diKeys);
	bool Pressed();


};
#endif // DBUTTON_H