#include "GlareEffect.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void GlareEffect::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource,
	uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {


	PostEffect::Initialize(dxCommon, srvManager, CSFilePath,
		inputResource, inputSrvIdx, outputResource);

	//グレアエフェクト情報用バッファ生成
	glareEffectInfoData_ = nullptr;
	TakeC::DirectXCommon::ComPtr<ID3D12Device> device = dxCommon_->GetDevice();
	glareEffectInfoResource_ = TakeC::DirectXCommon::CreateBufferResource(device.Get(), sizeof(GlareEffectInfo));
	//バッファマッピング
	glareEffectInfoResource_->Map(0, nullptr, (void**)&glareEffectInfoData_);


}

void GlareEffect::UpdateImGui() {

	ImGui::Begin("GlareEffect");
	ImGui::SliderFloat("Intensity", &glareEffectInfoData_->intensity, 0.0f, 5.0f);
	ImGui::SliderFloat("Threshold", &glareEffectInfoData_->threshold, 0.0f, 5.0f);
	ImGui::SliderInt("SampleCount", &glareEffectInfoData_->sampleCount, 1, 32);
	ImGui::End();
}

void GlareEffect::Dispatch() {


	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
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
	//filterInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gBoxFilterInfo"), glareEffectInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(TakeC::WinApp::kScreenWidth / 8, TakeC::WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
