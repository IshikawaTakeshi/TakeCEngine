#include "DepthBasedOutline.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

void DepthBasedOutline::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);

	//PSOの名前を設定
	computePSO_->SetComputePipelineName("DepthBasedOutline");

	//Bufferの名前を設定
	inputResource_->SetName(L"DepthBasedOutline::InputResource");
	outputResource_->SetName(L"DepthBasedOutline::_OutputResource");

	//アウトライン情報のリソースを作成
	outlineInfoResource_ = dxCommon->CreateBufferResourceUAV(dxCommon->GetDevice(), sizeof(DepthBasedOutlineInfo), dxCommon->GetCommandList());
	outlineInfoResource_->SetName(L"DepthBasedOutline::InfoResource");
	//mapping
	outlineInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&outlineInfoData_));

	//depthテクスチャリソースを取得
	depthTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(dxCommon_->GetDepthStencilResource().Get(), DXGI_FORMAT_R32_FLOAT, depthTextureSrvIndex_);

}

void DepthBasedOutline::UpdateImGui() {
#ifdef _DEBUG

#endif // _DEBUG

}

void DepthBasedOutline::DisPatch() {

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
	//OutlineInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(computePSO_->GetComputeBindResourceIndex("gOutlineInfo"), outlineInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}
