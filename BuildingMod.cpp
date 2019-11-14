#include "BuildingMod.h"

void BuildingMod::PrintMessagePrefix() {
	game->PrintMessage(L"[");
	game->PrintMessage(L"Building Mod", 135, 206, 250);
	game->PrintMessage(L"] ");
}

void BuildingMod::ToggleBuildMode() {
	buildMode = !buildMode;
	buildWindow->SetBuildMode(buildMode);
	wchar_t message[64];
	swprintf(message, L"Building mode %ls.\n", buildMode ? L"enabled" : L"disabled");

	PrintMessagePrefix();
	game->PrintMessage(message);
}

bool BuildingMod::InBuildMode() {
	return buildMode;
}

void BuildingMod::SelectBlock(cube::Block block, bool verbose) {
	currentBlock = block;
	buildWindow->UpdateBlockState(block);
	if (verbose) {
		u8 r, g, b, type;
		r = block.red;
		g = block.green;
		b = block.blue;
		type = block.type;
		PrintMessagePrefix();
		wchar_t response[256];
		if (type == 2) { //block is water
			game->PrintMessage(L"Selected water block.\n", 135, 206, 250);
		}
		else if (type == 3) { //block is a solid wet block
			game->PrintMessage(L"Selected ", r, g, b);
			game->PrintMessage(L"wet", 135, 206, 250);
			swprintf(response, L" block color %u %u %u.\n", r, g, b);
			game->PrintMessage(response, r, g, b);
		}
		else { //other, solid block
			swprintf(response, L"Selected block color %u %u %u.\n", r, g, b);
			game->PrintMessage(response, r, g, b);
		}
	}
}

void BuildingMod::SetBlockColor(cube::Block block, bool verbose) {
	cube::Block newBlock = currentBlock;
	newBlock.red = block.red;
	newBlock.green = block.green;
	newBlock.blue = block.blue;
	SelectBlock(newBlock, verbose);
}

bool BuildingMod::CanBuildUnderwater() {
	return buildUnderwater;
}

void BuildingMod::ToggleUnderwaterBuilding() {
	buildUnderwater = !buildUnderwater;
}

bool BuildingMod::ReadyToBreak() {
	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	u64 currentMS = (currentTime.wSecond * 1000) + currentTime.wMilliseconds;
	u64 breakMS = (lastBreakTime.wSecond * 1000) + lastBreakTime.wMilliseconds;

	return (currentMS - breakMS) > 150;
}

void BuildingMod::BreakAction() {
	GetSystemTime(&lastBreakTime);

	EnterCriticalSection(&game->world->zones_critical_section);
	LongVector3 hitLocation;
	if (game->TraceCrosshairToBlock(reachRange, false, &hitLocation, buildUnderwater)) {
		cube::Block airBlock;
		airBlock.red = 255;
		airBlock.green = 255;
		airBlock.blue = 255;
		airBlock.type = cube::Block::Type::Air;
		airBlock.breakable = 0;
		game->world->SetBlock(hitLocation, airBlock);

		EnterCriticalSection(&game->host.world.zones_critical_section);
		game->host.world.SetBlock(hitLocation, airBlock);
		LeaveCriticalSection(&game->host.world.zones_critical_section);
	}
	LeaveCriticalSection(&game->world->zones_critical_section);
}

bool BuildingMod::ReadyToPlace() {
	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	u64 currentMS = (currentTime.wSecond * 1000) + currentTime.wMilliseconds;
	u64 placeMS = (lastPlacementTime.wSecond * 1000) + lastPlacementTime.wMilliseconds;

	return (currentMS - placeMS) > 150;
}

void BuildingMod::PlaceAction() {
	GetSystemTime(&lastPlacementTime);

	EnterCriticalSection(&game->world->zones_critical_section);
	LongVector3 hitLocation;
	if (game->TraceCrosshairToBlock(reachRange, true, &hitLocation, buildUnderwater)) {

		// Teleport the player upwards if they're standing in the block they're trying to place.
		// This allows players to pillar up.
		LongVector3* playerDotLocation = &game->GetPlayer()->entity_data.position;
		LongVector3 playerBlockLocation(pydiv(playerDotLocation->x, cube::DOTS_PER_BLOCK),
			pydiv(playerDotLocation->y, cube::DOTS_PER_BLOCK),
			pydiv(playerDotLocation->z, cube::DOTS_PER_BLOCK));
		if (playerBlockLocation.x == hitLocation.x &&
			playerBlockLocation.y == hitLocation.y &&
			playerBlockLocation.z == hitLocation.z + 1) 
		{
			playerDotLocation->z += 1 * cube::DOTS_PER_BLOCK;
		}

		game->world->SetBlock(hitLocation, currentBlock);
		game->host.world.SetBlock(hitLocation, currentBlock);
	}
	LeaveCriticalSection(&game->world->zones_critical_section);
}

bool BuildingMod::ReadyToPick() {
	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	u64 currentMS = (currentTime.wSecond * 1000) + currentTime.wMilliseconds;
	u64 pickMS = (lastPickTime.wSecond * 1000) + lastPickTime.wMilliseconds;

	return (currentMS - pickMS) > 250;
}

void BuildingMod::PickAction() {
	GetSystemTime(&lastPickTime);

	EnterCriticalSection(&game->world->zones_critical_section);
	LongVector3 hitLocation;
	if (game->TraceCrosshairToBlock(reachRange, false, &hitLocation)) {
		cube::Block hitBlock = game->world->GetBlockInterpolated(hitLocation);
		if (hitBlock.type != cube::Block::Air) {
			SelectBlock(hitBlock);
		}

	}
	LeaveCriticalSection(&game->world->zones_critical_section);
}

bool BuildingMod::InOtherGUI() {
	cube::GUI* gui = &game->gui;
	plasma::Node* nodes[] = {
		gui->startmenu_node,
		gui->character_selection_node,
		gui->character_creation_node,
		gui->plasma_node_10,
		gui->plasma_node_15,
		gui->equipment_inventory_widget->node,
		gui->crafting_inventory_widget->node,
		gui->multiplayer_widget->node,
		gui->vendor_inventory_widget->node,
		gui->adaption_widget->node,
		gui->voxel_widget->node,
		gui->enchant_widget->node,
		gui->question_widget->node,
		gui->system_widget->node,
		gui->controls_widget->node,
		gui->options_widget->node
	};

	for (plasma::Node* node : nodes) {
		if (!node) continue;
		auto data = &node->display->visibility.data;
		auto frame = node->display->visibility.current_frame;

		if (data->at(frame)) {
			return true;
		}
	}

	if (gui->chat_widget->typebox_active) return true;
	if (gui->map_open) return true;
	if (gui->esc_menu_displayed) return true;

	return false;
}

void BuildingMod::PrintBlockInfo() {
	PrintMessagePrefix();
	wchar_t msg[256];
	swprintf(msg, L"R: %02X; G: %02X; B: %02X, Unk: %02X, type: %02X, breakable: %02X\n", 
		currentBlock.red, currentBlock.green, currentBlock.blue, currentBlock.field_3, currentBlock.type, currentBlock.breakable);
	game->PrintMessage(msg);
}

cube::Block BuildingMod::GetCurrentBlock() {
	return currentBlock;
}


// Event handlers
void BuildingMod::Initialize() {
	GetSystemTime(&lastPlacementTime);
	GetSystemTime(&lastBreakTime);
	GetSystemTime(&lastPickTime);
	currentBlock.red = 255;
	currentBlock.green = 255;
	currentBlock.blue = 255;
	currentBlock.type = cube::Block::Type::Solid;
	buildWindow = new BuildWindow(this);
}

void BuildingMod::OnGameTick(cube::Game* game) {
	this->game = game;
}

void BuildingMod::OnGetKeyboardState(BYTE* diKeys) {
	buildWindow->OnGetKeyboardState(diKeys);
	buildButton.Update(diKeys);
	if (buildButton.Pressed()) {
		ToggleBuildMode();
	}

}

void BuildingMod::OnGetMouseState(DIMOUSESTATE* diMouse) {
	buildWindow->OnGetMouseState(diMouse);
	if (buildMode && !InOtherGUI()) {

		if (diMouse->rgbButtons[0] && ReadyToBreak()) {
			BreakAction();
		}
		else if (diMouse->rgbButtons[1] && ReadyToPlace()) {
			PlaceAction();
		}
		else if (diMouse->rgbButtons[2] && ReadyToPick()) {
			PickAction();
		}

		diMouse->rgbButtons[0] = 0;
		diMouse->rgbButtons[1] = 0;
		diMouse->rgbButtons[2] = 0;
	}
}

void BuildingMod::OnPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	buildWindow->Present();
}

int BuildingMod::OnWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return buildWindow->WindowProc(hwnd, uMsg, wParam, lParam);
}

int BuildingMod::OnChat(std::wstring* msg) {
	if (!wcscmp(msg->c_str(), L"/build info")) {
		PrintBlockInfo();
		return 1;
	}
	return 0;
}