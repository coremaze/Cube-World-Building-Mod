#include "BuildingMod.h"
#include "BuildNetwork/BuildNetwork.h"

#define BLOCK_BREAK_WAIT_TIME 150
#define BLOCK_PICK_WAIT_TIME 250
#define BLOCK_PLACE_WAIT_TIME 150

#define MOD_DIR "Mods\\BuildingMod\\"
#define CONFIG_DIR MOD_DIR "config\\"
#define BUILDTOGGLE_CONFIG CONFIG_DIR "buildToggle"


void BuildingMod::PrintMessagePrefix() {
	game->PrintMessage(L"[");
	game->PrintMessage(L"Building Mod", 135, 206, 250);
	game->PrintMessage(L"] ");
}

void BuildingMod::ToggleBuildMode() {
	buildMode = !buildMode;
	buildWindow->SetBuildMode(buildMode);

	PrintMessagePrefix();
	game->PrintMessage(L"Building ");
	if (buildMode) {
		game->PrintMessage(L"enabled", 70, 255, 70);
	}
	else {
		game->PrintMessage(L"disabled", 255, 70, 70);
	}
	game->PrintMessage(L".\n");
}

bool BuildingMod::InBuildMode() {
	return buildMode;
}

void BuildingMod::SelectBlock(cube::Block block, bool verbose) {
	currentBlock = block;
	buildWindow->UpdateBlockState(block);
	if (verbose) {
		u8 r, g, b;
		cube::Block::Type type;
		r = block.red;
		g = block.green;
		b = block.blue;
		type = block.type;
		PrintMessagePrefix();
		wchar_t response[256];
		if (type == cube::Block::Water) {
			game->PrintMessage(L"Selected water block.\n", 135, 206, 250);
		}
		else if (type == cube::Block::Wet) {
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

	return (currentMS - breakMS) > BLOCK_BREAK_WAIT_TIME;
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
		buildNetwork->SendSingleBlockUpdate(game->client.host_steam_id, hitLocation, airBlock, false);
	}
	LeaveCriticalSection(&game->world->zones_critical_section);
}

bool BuildingMod::ReadyToPlace() {
	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	u64 currentMS = (currentTime.wSecond * 1000) + currentTime.wMilliseconds;
	u64 placeMS = (lastPlacementTime.wSecond * 1000) + lastPlacementTime.wMilliseconds;

	return (currentMS - placeMS) > BLOCK_PLACE_WAIT_TIME;
}

void BuildingMod::PlaceAction() {
	GetSystemTime(&lastPlacementTime);

	EnterCriticalSection(&game->world->zones_critical_section);
	LongVector3 hitLocation;
	if (game->TraceCrosshairToBlock(reachRange, true, &hitLocation, buildUnderwater)) {
		buildNetwork->SendSingleBlockUpdate(game->client.host_steam_id, hitLocation, currentBlock, false);
	}
	LeaveCriticalSection(&game->world->zones_critical_section);
}

bool BuildingMod::ReadyToPick() {
	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);
	u64 currentMS = (currentTime.wSecond * 1000) + currentTime.wMilliseconds;
	u64 pickMS = (lastPickTime.wSecond * 1000) + lastPickTime.wMilliseconds;

	return (currentMS - pickMS) > BLOCK_PICK_WAIT_TIME;
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
		gui->world_selection_node,
		gui->world_creation_node,
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

void BuildingMod::QueueBlock(cube::Block& block, LongVector3& position) {
	IntVector2 chunkPosition(pydiv(position.x, cube::BLOCKS_PER_ZONE), pydiv(position.y, cube::BLOCKS_PER_ZONE));
	cube::Zone* zone = game->world->GetZone(chunkPosition);
	if (!zone) return;
	chunkUpdatesMtx.lock();
	zone->chunk.Remesh();
	chunkUpdates.push_back(ChunkUpdate(block, position));
	chunkUpdatesMtx.unlock();
}

void BuildingMod::PlaceSingleBlock(cube::Block& block, LongVector3& position) {
	LongVector3 soundPos = position * cube::DOTS_PER_BLOCK;
	cube::Block::Type blockType = block.type;

	if (block.type == cube::Block::Air) {
		blockType = game->world->GetBlockInterpolated(position).type;
	}

	switch (blockType) {
		case cube::Block::Wet: {
			cube::GetGame()->PlaySoundEffect(cube::Game::sound_arrow_destroy, soundPos, 0.10, ((float)rand() / RAND_MAX) + 1.0);
			// This fallthrough is deliberate. The water sound effect should play on top.
		}
		case cube::Block::Water: {
			cube::Game::SoundEffect choices[] = { cube::Game::sound_step_water, cube::Game::sound_step_water2, cube::Game::sound_step_water3 };
			cube::Game::SoundEffect choice = choices[rand() % (sizeof(choices) / sizeof(*choices))];
			cube::GetGame()->PlaySoundEffect(choice, soundPos, 0.20, ((float)rand() / RAND_MAX) + 1.0);
			break;
		}
		case cube::Block::Lava: {
			cube::GetGame()->PlaySoundEffect(cube::Game::sound_step_water, soundPos, 0.20, ((float)rand() / RAND_MAX) / 3 + 0.3);
			cube::GetGame()->PlaySoundEffect(cube::Game::sound_fireball, soundPos, 0.05, ((float)rand() / RAND_MAX) + 1.0);
			break;
		}
		case cube::Block::Poison: {
			cube::GetGame()->PlaySoundEffect(cube::Game::sound_step_water, soundPos, 0.30, ((float)rand() / RAND_MAX) / 4 + 0.2);
			break;
		}
		case cube::Block::Air: {
			break;
		}
		default: {
			cube::GetGame()->PlaySoundEffect(cube::Game::sound_arrow_destroy, soundPos, 0.15, ((float)rand() / RAND_MAX) + 1.0);
			break;
		}
	}

	QueueBlock(block, position);
}

void BuildingMod::QueueBlocks(std::vector<std::pair<cube::Block, LongVector3>> blocks) {
	chunkUpdatesMtx.lock();
	for (auto& p : blocks) {
		cube::Block& block = p.first;
		LongVector3& position = p.second;
		IntVector2 chunkPosition(pydiv(position.x, cube::BLOCKS_PER_ZONE), pydiv(position.y, cube::BLOCKS_PER_ZONE));
		cube::Zone* zone = game->world->GetZone(chunkPosition);
		if (!zone) continue;
		chunkUpdates.push_back(ChunkUpdate(block, position));
		zone->chunk.Remesh();
	}
	chunkUpdatesMtx.unlock();
}

void BuildingMod::ReloadZonesIfNeeded() {
	static bool firstTime = true;
	if (oldHostSteamID != game->client.host_steam_id) {
		oldHostSteamID = game->client.host_steam_id;
		if (!firstTime) {
			// There's no need to reload if it's the first time it has happened.
			cube::World* world = game->world;
			EnterCriticalSection(&world->zones_critical_section);
			EnterCriticalSection(&world->zones_mesh_critical_section);
			std::vector<cube::Zone*>* zones = new std::vector<cube::Zone*>();
			for (auto pair : world->zones) {
				cube::Zone* zone = pair.second;
				zones->push_back(zone);
			}
			world->zones.clear();
			LeaveCriticalSection(&world->zones_mesh_critical_section);
			LeaveCriticalSection(&world->zones_critical_section);
			// Can't delete zones now because the D3D11Renderer is still using them.
			// Wait some sane amount of time and create a stupid fucking race condition.
			// Modding this game is such a mess.
			std::thread(
				[](std::vector<cube::Zone*>* zones) {
					Sleep(4000);
					for (cube::Zone* zone : *zones) {
						delete zone;
					}
					delete zones;
				},
				zones).detach();
		}
		firstTime = false;
		
		PrintMessagePrefix();
		game->PrintMessage(L"All chunks are currently being updated. Block updates will be slow until this process is finished.\n");
	}
}

ZoneSaver::WorldContainer* BuildingMod::GetWorldContainer() {
	return &this->worldContainer;
}

void BuildingMod::PlayGUISelectNoise() {
	game->PlaySoundEffect(cube::Game::sound_menu_select, 1.0, 0.8);
}

DButton* BuildingMod::GetBuildButton() {
	return &buildButton;
}

void BuildingMod::LoadButtonConfig() {
	CreateDirectory(MOD_DIR, NULL);
	CreateDirectory(CONFIG_DIR, NULL);
	std::ifstream file(BUILDTOGGLE_CONFIG, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		//File exists, read it
		size_t fsize = file.tellg();
		if (fsize == sizeof(buildButton.diKey)) {
			file.seekg(0, std::ios::beg);
			file.read((char*)&buildButton.diKey, fsize);
		}
		file.close();
	}
	SaveButtonConfig();
}

void BuildingMod::SaveButtonConfig() {
	CreateDirectory(MOD_DIR, NULL);
	CreateDirectory(CONFIG_DIR, NULL);
	std::ofstream file;
	file.open(BUILDTOGGLE_CONFIG, std::ios::out | std::ios::binary);
	file.write((char*)&buildButton.diKey, sizeof(buildButton.diKey));
	file.close();
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
	buildNetwork = new BuildNetwork(this);
	LoadButtonConfig();
}

void BuildingMod::OnGameTick(cube::Game* game) {
	
	this->game = game;
	buildNetwork->Process();
	ReloadZonesIfNeeded();
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

int BuildingMod::OnChat(std::wstring* message) {
	if (!wcscmp(message->c_str(), L"/build info")) {
		PrintBlockInfo();
		return 1;
	}
	return 0;
}

void BuildingMod::OnZoneGenerated(cube::Zone* zone) {
	if (zone->world != game->world) {
		return;
	}

	buildNetwork->SendZoneLoad(game->client.host_steam_id, zone->position);
}

void BuildingMod::OnChunkRemesh(cube::Zone* zone) {
	chunkUpdatesMtx.lock();
	EnterCriticalSection(&game->world->zones_critical_section);
	EnterCriticalSection(&game->world->zones_mesh_critical_section);
	LongVector3* playerDotLocation = &game->GetPlayer()->entity_data.position;
	LongVector3 playerBlockLocation(pydiv(playerDotLocation->x, cube::DOTS_PER_BLOCK),
									pydiv(playerDotLocation->y, cube::DOTS_PER_BLOCK),
									pydiv(playerDotLocation->z, cube::DOTS_PER_BLOCK));

	bool playerAdjusted = false;
	for (auto chunkUpdate = chunkUpdates.begin(); chunkUpdate != chunkUpdates.end(); ) {
		if (pydiv(chunkUpdate->position.x, cube::BLOCKS_PER_ZONE) == zone->position.x && pydiv(chunkUpdate->position.y, cube::BLOCKS_PER_ZONE) == zone->position.y) {
			// Teleport the player upwards if they're standing in the block they're trying to place.
			// This allows players to pillar up.
			if (!playerAdjusted &&
				playerBlockLocation.x == chunkUpdate->position.x && 
				playerBlockLocation.y == chunkUpdate->position.y && 
				playerBlockLocation.z == chunkUpdate->position.z + 1) {
				playerDotLocation->z += 1 * cube::DOTS_PER_BLOCK;
				playerAdjusted = true;
			}
			game->world->SetBlock(chunkUpdate->position, chunkUpdate->block, true);
			chunkUpdate = chunkUpdates.erase(chunkUpdate);
		}
		else {
			chunkUpdate++;
		}
	}

	LeaveCriticalSection(&game->world->zones_mesh_critical_section);
	LeaveCriticalSection(&game->world->zones_critical_section);
	chunkUpdatesMtx.unlock();
	
	return;
}
