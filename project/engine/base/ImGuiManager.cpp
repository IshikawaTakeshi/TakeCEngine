#include "ImGuiManager.h"

#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <wrl.h>
#include <cassert>

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
		dxCommon_->GetRtvFormat(),
		srvManager_->GetSrvHeap(),
		srvManager_->GetSrvDescriptorHandleCPU(useSrvIndex),
		srvManager_->GetSrvDescriptorHandleGPU(useSrvIndex)
	);
}

void ImGuiManager::Finalize() {

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

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

void ImGuiManager::Draw() {

	ID3D12GraphicsCommandList* commandList =  dxCommon_->GetCommandList();
	ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetSrvHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}
