#include "DepthBasedOutline.h"
#include "engine/Utility/ResourceBarrier.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include <cassert>

//=============================================================================
// 初期化
//=============================================================================
void DepthBasedOutline::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);

	//PSOの名前を設定
	computePSO_->SetComputePipelineName("DepthBasedOutline");

	//Bufferの名前を設定
	inputResource_->SetName(L"DepthBasedOutline::InputResource");
	outputResource_->SetName(L"DepthBasedOutline::_OutputResource");

	//depthテクスチャリソースを取得
	depthTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(dxCommon_->GetDepthStencilResource().Get(), depthTextureSrvIndex_);

	//アウトライン情報のリソースを作成
	outlineInfoResource_ = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(DepthBasedOutlineInfo));
	outlineInfoResource_->SetName(L"DepthBasedOutline::InfoResource");
	outlineInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&outlineInfoData_));

	outlineInfoData_->weight = 0.1f;                           // 初期値の設定
	outlineInfoData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 初期色の設定
	outlineInfoData_->isActive = true;                         // アウトラインを有効にする
	outlineInfoData_->distantSensitivity = 0.0f;               //遠方オブジェクトの感度調整係数
	outlineInfoData_->distantStart = 200.0f;                   //遠方補正を始めるviewZ
	outlineInfoData_->distantEnd = 2200.0f;                    //補正を最大にするviewZ
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void DepthBasedOutline::UpdateImGui() {
#ifdef _DEBUG

	if (ImGui::TreeNode("DepthBasedOutline")) {
		ImGui::SliderFloat("weight", &outlineInfoData_->weight, 0.0f, 10.0f);
		ImGui::ColorEdit4("Color", &outlineInfoData_->color.x);
		ImGui::SliderFloat("distantSensitivity", &outlineInfoData_->distantSensitivity, 0.0f, 1.0f);
		ImGui::SliderFloat("distantStart", &outlineInfoData_->distantStart, 0.0f, 500.0f);
		ImGui::SliderFloat("distantEnd", &outlineInfoData_->distantEnd, 0.0f, 5000.0f);
		ImGui::TreePop();
	}
	ImGui::SameLine();
	ImGui::Checkbox("##DepthBaseoutLine::isActive", &outlineInfoData_->isActive);
#endif // _DEBUG

}

//=============================================================================
// Dispatch
//=============================================================================
void DepthBasedOutline::Dispatch() {

	if(!outlineInfoData_->isActive) {
		return; // アウトラインが無効な場合は処理をスキップ
	}

	//outputTexure
	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	//depthTexture
	//DEPTH_WRITE >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		dxCommon_->GetDepthStencilResource().Get());

	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	//inputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	//outputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);
	//depthTexture
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gDepthTexture"), depthTextureSrvIndex_);
	//OutlineInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gOutlineInfo"),
		outlineInfoResource_->GetGPUVirtualAddress());
	//cameraInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gCameraInfo"),
		CameraManager::GetInstance().GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	//depthTexture
	//NON_PIXEL_SHADER_RESOURCE >> DEPTH_WRITE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		dxCommon_->GetDepthStencilResource().Get());

	//outputTexure
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
