#include "BoxFilter.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

using namespace TakeC;

//=============================================================================
// 初期化
//=============================================================================
void BoxFilter::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx,outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("BoxFilterPSO");
	//Bufferの名前付け
	inputResource_->SetName(L"BoxFilter::inputResource_");
	outputResource_->SetName(L"BoxFilter::outputResource_");

	//SRVの生成
	filterInfoResource_ = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(BoxFilterInfo));
	filterInfoResource_->SetName(L"BoxFilter::filterInfoResource_");
	filterInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&filterInfoData_));
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void BoxFilter::UpdateImGui() {
#ifdef _DEBUG
	ImGui::Text("BoxFilter");
	ImGui::SameLine();
	ImGui::Checkbox("##BoxFilter::isActive", &filterInfoData_->isActive);
#endif // _DEBUG
}

//=============================================================================
// Dispatch
//=============================================================================
void BoxFilter::Dispatch() {

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
		computePSO_->GetComputeBindResourceIndex("gBoxFilterInfo"), filterInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}