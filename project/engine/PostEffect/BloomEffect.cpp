#include "BloomEffect.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void BloomEffect::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx,outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("BloomEffectPSO");
	//Bufferの名前付け
	inputResource_->SetName(L"Bloom::inputResource_");
	outputResource_->SetName(L"Bloom::outputResource_");

	effectInfoResource_ = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(BloomEffectInfo));
	effectInfoResource_->SetName(L"Bloom::effectInfoResource_");
	effectInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&effectInfoData_));

	effectInfoData_->isActive = true;
	effectInfoData_->threshold = 0.75f;
	effectInfoData_->strength = 0.15f;
	effectInfoData_->sigma = 0.05f;
}

void BloomEffect::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Text("BloomEffect");
	ImGui::SameLine();
	ImGui::SliderFloat("threshold", &effectInfoData_->threshold, 0.0f, 1.0f);
	ImGui::SliderFloat("strength", &effectInfoData_->strength, 0.0f, 0.5f);
	ImGui::SliderFloat("sigma", &effectInfoData_->sigma, 0.0f, 2.0f);
	ImGui::Checkbox("isActive", &effectInfoData_->isActive);
#endif // _DEBUG

}

void BloomEffect::DisPatch() {

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
	//effectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gBloomEffectInfo"), effectInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
