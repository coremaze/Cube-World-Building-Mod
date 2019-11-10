#include "BuildWindow.h"
#include "BuildingMod.h"


BuildWindow::BuildWindow(BuildingMod* mod) {
	this->mod = mod;
}

void BuildWindow::Present() {
	if (!initialized) {
		Initialize();
	}

	if (!game->gui.esc_menu_displayed) {
		wantMouse = false;
		wantKeyboard = false;
		return;
	}

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.50f, 0.30f, 0.30f, 1.00f);
	ImGuiIO& io = ImGui::GetIO();
	wantMouse = io.WantCaptureMouse;
	wantKeyboard = io.WantCaptureKeyboard;
	io.MouseDrawCursor = wantMouse;
	io.Fonts->AddFontFromFileTTF("resource1.dat", 16.0f);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImVec2 size(360, 170);
	ImGui::SetNextWindowSize(size);
	ImGui::Begin("Building Mod", nullptr, size, -1.0, ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Build mode", &buildModeEnabled);

	ImGui::Separator();

	cube::Block currentBlock = mod->GetCurrentBlock();

	ImGui::Checkbox("Water", &waterBlock);
	if (waterBlock) {
		currentBlock.type = cube::Block::Type::Water;
		wetBlock = false;
		lavaBlock = false;
	}

	ImGui::Checkbox("Wet", &wetBlock);
	if (wetBlock) {
		currentBlock.type = cube::Block::Type::Water | cube::Block::Type::Colored;
		waterBlock = false;
		lavaBlock = false;
	} 
	
	ImGui::Checkbox("Lava", &lavaBlock);
	if (lavaBlock) {
		currentBlock.type = cube::Block::Type::Lava | cube::Block::Type::Colored;
		waterBlock = false;
		wetBlock = false;
	}

	if (!waterBlock && !wetBlock) {
		currentBlock.type &= ~cube::Block::Type::Water;
	}

	if (!waterBlock) {
		ImVec4 blockColor;
		blockColor.x = currentBlock.red / 255.0;
		blockColor.y = currentBlock.green / 255.0;
		blockColor.z = currentBlock.blue / 255.0;
		blockColor.w = 1.0;

		if (ImGui::ColorEdit3("Block Color", (float*)&blockColor)) {
			currentBlock.red = blockColor.x * 255.0;
			currentBlock.green = blockColor.y * 255.0;
			currentBlock.blue = blockColor.z * 255.0;
		}

		currentBlock.type |= cube::Block::Type::Colored;
		
	}

	mod->SelectBlock(currentBlock, false);

	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	Update();
}

void BuildWindow::Initialize() {
	game = cube::GetGame();
	initialized = true;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(GetActiveWindow());
	ImGui_ImplDX11_Init(cube::GetID3D11Device(), cube::GetID3D11DeviceContext());
}

void BuildWindow::Update() {
	if (buildModeEnabled != mod->InBuildMode()) {
		mod->ToggleBuildMode();
	}
	UpdateBlockState(mod->GetCurrentBlock());
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int BuildWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (wantMouse) {
		switch (msg) {
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
				return 1;
		}
	}
	if (wantKeyboard) return 1;
	return 0;
}

void BuildWindow::SetBuildMode(bool mode) {
	buildModeEnabled = mode;
}

void BuildWindow::OnGetMouseState(DIMOUSESTATE* diMouse) {
	if (wantMouse) {
		diMouse->rgbButtons[0] = 0;
		diMouse->rgbButtons[1] = 0;
		diMouse->rgbButtons[2] = 0;
	}
}

void BuildWindow::OnGetKeyboardState(BYTE* diKeys) {
	if (wantKeyboard) {
		memset(diKeys, 0, 256);
	}
}

void BuildWindow::UpdateBlockState(cube::Block block) {
	wetBlock = (block.type & cube::Block::Type::Water) && (block.type & cube::Block::Type::Colored);
	waterBlock = (block.type & cube::Block::Type::Water) && !wetBlock;
	lavaBlock =(block.type & cube::Block::Type::Lava) && (block.type & cube::Block::Type::Colored);
}