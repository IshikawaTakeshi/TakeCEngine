#include "DepthBasedOutline.h"
#include "engine/Utility/ResourceBarrier.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include <cassert>

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

	outlineInfoData_->weight = 0.1f; // 初期値の設定
	outlineInfoData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 初期色の設定
	outlineInfoData_->isActive = true; // アウトラインを有効にする
}

void DepthBasedOutline::UpdateImGui() {
#ifdef _DEBUG

	if (ImGui::TreeNode("DepthBasedOutline")) {
		ImGui::SliderFloat("weight", &outlineInfoData_->weight, 0.0f, 10.0f);
		ImGui::ColorEdit4("Color", &outlineInfoData_->color.x);
		ImGui::TreePop();
	}
	ImGui::SameLine();
	ImGui::Checkbox("##DepthBaseoutLine::isActive", &outlineInfoData_->isActive);
#endif // _DEBUG

}

void DepthBasedOutline::DisPatch() {

	if(!outlineInfoData_->isActive) {
		return; // アウトラインが無効な場合は処理をスキップ
	}

	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

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
		CameraManager::GetInstance()->GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress());

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
