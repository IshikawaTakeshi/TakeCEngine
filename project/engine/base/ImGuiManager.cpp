#include "ImGuiManager.h"
#include "engine/Base/TextureManager.h"
#include <wrl.h>
#include <cassert>
#include <format>

//====================================================================
//			初期化
//====================================================================
void TakeC::ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//コンテキストの生成
	ImGui::CreateContext();
	//ImGuiのスタイルの設定
	ApplyStyle();
	//ImGuiのWin32の初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());

	//デスクリプタヒープの設定
	uint32_t useSrvIndex = srvManager_->Allocate();

	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(),
		static_cast<int>(dxCommon_->GetBufferCount()),
		dxCommon_->GetRtvManager()->GetRtvDesc().Format,
		srvManager_->GetSrvHeap(),
		srvManager_->GetSrvDescriptorHandleCPU(useSrvIndex),
		srvManager_->GetSrvDescriptorHandleGPU(useSrvIndex)
	);

	//ImGuiの設定
	ImGuiIO& io = ImGui::GetIO();

	// Dockingを有効にする
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 
	// デフォルトのフォントを設定
	io.Fonts->AddFontDefault();
	//ウィンドウ全体に描画できるようにする
	io.DisplaySize = ImVec2(static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight));
}

//====================================================================
//			終了処理
//====================================================================
void TakeC::ImGuiManager::Finalize() {

	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGuiコンテキストの破棄
	ImGui::DestroyContext();
}

//====================================================================
// ImGui受付開始・終了
//====================================================================
void TakeC::ImGuiManager::Begin() {

	//ImGui受付開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void TakeC::ImGuiManager::End() {
	//ImGuiの内部コマンドを生成する
	ImGui::Render();
}

//====================================================================
// DockSpaceの作成
//====================================================================
void TakeC::ImGuiManager::CreateDockSpace() {

	const ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(vp->WorkSize, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags hostFlags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("DockSpaceHost", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	// DockSpace（ここがドック先の“本体”）
	ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
	ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_None;

	// これをONにしていると「中央ノードが特殊扱い」になり、挙動が制限されることがあるので
	// まずはOFF推奨（必要になったら後で調整）
	//dockFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

	ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockFlags);

	// メニューバー（任意）
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Docking")) {
			ImGui::TextUnformatted("Drag any window tab into the Viewport area.");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End(); // DockSpaceHost
}

//====================================================================
// デバッグ画面の描画
//====================================================================
void TakeC::ImGuiManager::DrawDebugScreen() {

	ImGui::Begin("Viewport");
	{
		// Viewport内にレンダー結果を表示
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		ImGui::Image(
			ImTextureID(srvManager_->GetSrvDescriptorHandleGPU(renderTextureIndex_).ptr),
			avail,
			ImVec2(0, 0),
			ImVec2(1, 1),
			ImVec4(1, 1, 1, 1)
		);
	}
	ImGui::End();
}

//====================================================================
// ImGuiのスタイル適用
//====================================================================
void TakeC::ImGuiManager::ApplyStyle() {

	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.05f, 0.05f, 0.053f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Border
	colors[ImGuiCol_Border] = ImVec4{ 0.54f, 0.37f, 0.71f, 0.29f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

	// Text
	colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

	// Popups
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.05f, 0.05f, 0.053f, 0.92f };

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.1f, 0.1f, 0.2f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

	// Seperator
	colors[ImGuiCol_Separator] = ImVec4{ 0.54f, 0.17f, 0.71f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

	// Docking
	colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

	auto& style = ImGui::GetStyle();
	style.TabRounding = 4;
	style.ScrollbarRounding = 9;
	style.WindowRounding = 7;
	style.GrabRounding = 3;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ChildRounding = 4;
}

//====================================================================
// string用コンボボックス
//====================================================================
bool TakeC::ImGuiManager::ComboBoxString(const char* label, std::vector<std::string>& items, int& currentIndex) {
	// 値変更フラグ
	bool clicked = false;

	if (items.empty()) return false;

	// 未選択状態を許可（-1）
	if (currentIndex < -1 || currentIndex >= static_cast<int>(items.size())) {
		currentIndex = -1;
	}

	// 未選択時の表示名
	const char* preview = (currentIndex >= 0) ? items[currentIndex].c_str() : "Select...";

	if (ImGui::BeginCombo(label, preview)) {
		for (int i = 0; i < static_cast<int>(items.size()); ++i) {
			const bool isSelected = (currentIndex == i);
			if (ImGui::Selectable(items[i].c_str(), isSelected)) {
				currentIndex = i;
				clicked = true; // 同じ要素でも「クリックした事実」で true にするならここで常に true
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	return clicked;
}

//====================================================================
// テクスチャセレクターの表示
//====================================================================
bool TakeC::ImGuiManager::TextureSelector(const char* label, std::string& currentFilePath) {
	bool isChanged = false;

	// Header
	if (ImGui::CollapsingHeader(label)) {

		// Reload check (optional but good)
		TextureManager::GetInstance().CheckAndReloadTextures();

		// Search
		static char searchBuffer[64] = "";
		ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));

		// List
		auto textureFileNames = TextureManager::GetInstance().GetLoadedNonCubeTextureFileNames();

		// Grid setup
		const float thumbSize = 64.0f;
		const float padding = 8.0f;
		const float cellSize = thumbSize + padding;
		const float availWidth = ImGui::GetContentRegionAvail().x;
		const int columns = std::max(1, int(availWidth / cellSize));

		ImGui::BeginChild("TextureSelectorGrid", ImVec2(0, 300), true);

		int col = 0;
		for (const auto& file : textureFileNames) {
			// Search filter
			if (searchBuffer[0] != '\0' && file.find(searchBuffer) == std::string::npos) {
				continue;
			}

			// Draw logic
			auto handle = TextureManager::GetInstance().GetSrvHandleGPU(file);
			ImTextureID texID = (ImTextureID)handle.ptr;

			bool isSelected = (currentFilePath == file);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
			}

			ImGui::PushID(file.c_str());
			if (ImGui::ImageButton(file.c_str(), texID, ImVec2(thumbSize, thumbSize))) {
				currentFilePath = file;
				isChanged = true;
			}
			ImGui::PopID();

			if (isSelected) {
				ImGui::PopStyleColor();
				// Draw rect...
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 min = ImGui::GetItemRectMin();
				ImVec2 max = ImGui::GetItemRectMax();
				drawList->AddRect(min, max, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);
			}

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", file.c_str());
			}

			col++;
			if (col < columns) ImGui::SameLine(); else col = 0;
		}
		ImGui::EndChild();
	}
	return isChanged;
}

//====================================================================
// ImGui描画後処理
//====================================================================
void TakeC::ImGuiManager::PostDraw() {

	ID3D12GraphicsCommandList* commandList =  dxCommon_->GetCommandList();

	//実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

//====================================================================
// 各種デバッグ表示用関数
//====================================================================
void TakeC::ImGuiManager::QuaternionScreenPrintf(const std::string& label, const Quaternion& q) {
	// ImGuiでクォータニオンを画面表示
	ImGui::Text("%s : %.2f %.2f %.2f %.2f", label.c_str(), q.x, q.y, q.z, q.w);
}

void TakeC::ImGuiManager::Vector3ScreenPrintf(const std::string& label, const Vector3& v) {
	// ImGuiでベクトルを画面表示
	ImGui::Text("%s : %.2f %.2f %.2f", label.c_str(), v.x, v.y, v.z);
}

void TakeC::ImGuiManager::Matrix4x4ScreenPrintf(const std::string& label, const Matrix4x4& m) {
	// ImGuiで4x4行列を画面表示
	ImGui::Text(label.c_str());
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
}
