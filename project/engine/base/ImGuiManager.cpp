#include "ImGuiManager.h"

#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <wrl.h>
#include <cassert>
#include <format>

//====================================================================
//			初期化
//====================================================================
void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//コンテキストの生成
	ImGui::CreateContext();
	//ImGuiのスタイルの設定
	ImGui::StyleColorsDark();
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
void ImGuiManager::Finalize() {

	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGuiコンテキストの破棄
	ImGui::DestroyContext();
}

//====================================================================
// ImGui受付開始・終了
//====================================================================
void ImGuiManager::Begin() {

	//ImGui受付開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {
	//ImGuiの内部コマンドを生成する
	ImGui::Render();
}

//====================================================================
// デバッグ画面の描画
//====================================================================
void ImGuiManager::DrawDebugScreen() {
	ImGui::SetNextWindowBgAlpha(1.0f); // 完全透明
	ImGui::Begin("ImGuiManager::DebugScreen");
	//レンダーターゲットの内容を表示
	ImGui::Image(
		ImTextureID(srvManager_->GetSrvDescriptorHandleGPU(renderTextureIndex_).ptr),
		ImVec2(static_cast<float>(WinApp::kScreenWidth), static_cast<float>(WinApp::kScreenHeight)),
		ImVec2(0, 0), // UV座標の左上
		ImVec2(1, 1),  // UV座標の右下
		ImVec4(1, 1, 1, 1) // 色の設定（白）
	);
	ImGui::End();
}

//====================================================================
// ImGui描画後処理
//====================================================================
void ImGuiManager::PostDraw() {

	ID3D12GraphicsCommandList* commandList =  dxCommon_->GetCommandList();

	//実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

//====================================================================
// 各種デバッグ表示用関数
//====================================================================
void ImGuiManager::QuaternionScreenPrintf(const std::string& label, const Quaternion& q) {
	// ImGuiでクォータニオンを画面表示
	ImGui::Text("%s : %.2f %.2f %.2f %.2f", label.c_str(), q.x, q.y, q.z, q.w);
}

void ImGuiManager::Vector3ScreenPrintf(const std::string& label, const Vector3& v) {
	// ImGuiでベクトルを画面表示
	ImGui::Text("%s : %.2f %.2f %.2f", label.c_str(), v.x, v.y, v.z);
}

void ImGuiManager::Matrix4x4ScreenPrintf(const std::string& label, const Matrix4x4& m) {
	// ImGuiで4x4行列を画面表示
	ImGui::Text(label.c_str());
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
	ImGui::Text("%.3f %.3f %.3f %.3f", m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
}
