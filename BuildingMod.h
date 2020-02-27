#ifndef BUILDINGMOD_H
#define BUILDINGMOD_H
#include "cwmods/cwsdk.h"
#include "DButton.h"
#include "BuildWindow.h"
#include "ChunkUpdate.h"
#include <mutex>
#include <vector>
#include <map>
#include "ZoneSaver/ZoneSaver.h"

class BuildNetwork;
class BuildingMod : GenericMod {
private:
	DButton buildButton = DButton(DIK_GRAVE);
	SYSTEMTIME lastPlacementTime;
	SYSTEMTIME lastBreakTime;
	SYSTEMTIME lastPickTime;
	bool buildMode = false;
	cube::Game* game;
	f32 reachRange = 40.0;
	cube::Block currentBlock;
	bool buildUnderwater = false;
	CSteamID oldHostSteamID;
	BuildWindow* buildWindow;
	BuildNetwork* buildNetwork;
	std::list<ChunkUpdate> chunkUpdates;
	std::mutex chunkUpdatesMtx;
	
	ZoneSaver::WorldContainer worldContainer;

public:
	bool InBuildMode();
	void ToggleBuildMode();
	cube::Block GetCurrentBlock();
	void SelectBlock(cube::Block block, bool verbose = true);
	void SetBlockColor(cube::Block block, bool verbose = true);
	bool CanBuildUnderwater();
	void ToggleUnderwaterBuilding();
	void QueueBlock(cube::Block& block, LongVector3& position);
	void PlaceSingleBlock(cube::Block& block, LongVector3& position);
	void QueueBlocks(std::vector<std::pair<cube::Block, LongVector3>> blocks);
	ZoneSaver::WorldContainer* GetWorldContainer();
	void PlayGUISelectNoise();
	DButton* GetBuildButton();
	void LoadButtonConfig();
	void SaveButtonConfig();

private:
	void PrintMessagePrefix();
	bool ReadyToBreak();
	void BreakAction();
	bool ReadyToPlace();
	void PlaceAction();
	bool ReadyToPick();
	void PickAction();
	bool InOtherGUI();
	void PrintBlockInfo();
	void ReloadZonesIfNeeded();

	// Event handlers
	virtual void Initialize() override;
	virtual void OnGameTick(cube::Game* game) override;
	virtual void OnGetKeyboardState(BYTE* diKeys) override;
	virtual void OnGetMouseState(DIMOUSESTATE* diMouse) override;
	virtual void OnPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) override;
	virtual int OnWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual int OnChat(std::wstring* msg) override;
	virtual void OnZoneGenerated(cube::Zone* zone) override;
	virtual void OnZoneDestroy(cube::Zone* zone) override;
	virtual void OnChunkRemesh(cube::Zone* zone) override;
};

#endif
