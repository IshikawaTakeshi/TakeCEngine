#include "LuminanceBasedOutline.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void LuminanceBasedOutline::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("LuminanceBasedOutlinePSO");
	//Bufferの名前付け
	inputResource_->SetName(L"LuminanceBasedOutline::inputResource_");
	outputResource_->SetName(L"LuminanceBasedOutline::outputResource_");

	//アウトライン情報の初期化
	outlineInfoResource_ = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(LuminanceBasedOutlineInfo));
	outlineInfoResource_->SetName(L"LuminanceBasedOutline::outlineInfoResource_");
	outlineInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&outlineInfoData_));

	outlineInfoData_->weight = 6.0f; // 輪郭の強さ
	outlineInfoData_->isActive = true; // アウトラインの有効無効
	outlineInfoData_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // アウトラインの色
}

void LuminanceBasedOutline::UpdateImGui() {

#ifdef _DEBUG
	if (ImGui::TreeNode("LuminanceBasedOutline")) {
		ImGui::SliderFloat("weight", &outlineInfoData_->weight, 0.0f, 10.0f);
		ImGui::ColorEdit4("Color", &outlineInfoData_->color.x);
		ImGui::TreePop();
	}

	ImGui::SameLine();
	ImGui::Checkbox("##LumiBaseOutLine::isActive", &outlineInfoData_->isActive);
	
#endif // _DEBUG
}

void LuminanceBasedOutline::DisPatch() {

	if (!outlineInfoData_->isActive) {
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
	//outlineInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gOutlineInfo"), outlineInfoResource_->GetGPUVirtualAddress());

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
