#ifndef BUILDWINDOW_H
#define BUILDWINDOW_H

#include "cwmods/cwmods.h"
#include "imgui-1.73/imgui.h"
#include "imgui-1.73/examples/imgui_impl_dx11.h"
#include "imgui-1.73/examples/imgui_impl_win32.h"


class BuildingMod;
class BuildWindow {
	bool buildModeEnabled = false;

	bool waterBlock = false;
	bool wetBlock = false;
	bool lavaBlock = false;
	bool poisonBlock = false;
	bool underwater = false;

	bool wantMouse = false;
	bool wantKeyboard = false;
	bool initialized = false;

	cube::Game* game;
	BuildingMod* mod;
public:
	BuildWindow(BuildingMod* mod);
	void Present();
	void Initialize();
	void Update();
	int WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetBuildMode(bool mode);
	void OnGetMouseState(DIMOUSESTATE* diMouse);
	void OnGetKeyboardState(BYTE* diKeys);
	void UpdateBlockState(cube::Block block);
	void DeselectSpecialTypesExcept(bool& option);
	bool IsSpecialTypeSelected();

};

#endif // BUILDWINDOW_H