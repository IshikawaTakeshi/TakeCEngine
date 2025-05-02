#include "PostEffect.h"
#include "Utility/ResourceBarrier.h"
#include <cassert>

void PostEffect::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager,const std::wstring& CSFilePath,const Vector4& clearColor) {

	dxCommon_ = dxCommon;
	//SRVManagerの取得
	srvManager_ = srvManager;

	csFilePath_ = CSFilePath;

	//PSO初期化
	computePSO_ = std::make_unique<PSO>();
	computePSO_->CompileComputeShader(dxCommon_->GetDXC(), csFilePath_);
	computePSO_->CreateComputePSO(dxCommon_->GetDevice());

	rootSignature_ = computePSO_->GetComputeRootSignature();

	//inputRenderTexture
	inputResource_ = dxCommon_->CreateRenderTextureResource(
		dxCommon_->GetDevice(),WinApp::kClientWidth,WinApp::kClientHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,clearColor);

	//outputRenderTexture
	outputResource_ = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(),WinApp::kClientWidth,WinApp::kClientHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	//SRVの生成
	inputTexSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(inputResource_.Get(), inputTexSrvIndex_);

	//UAVの生成
	outputTexUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforRenderTexture(outputResource_.Get(), outputTexUavIndex_);
}

void PostEffect::DisPatch(uint32_t outputTextureUavIndex) {

	//RENDER_TARGET >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		inputResource_.Get());

	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	//inputTex
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), srvManager_->GetSrvDescriptorHandleGPU(inputTexSrvIndex_));
	//outputTex
	dxCommon_->GetCommandList()->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), srvManager_->GetSrvDescriptorHandleGPU(outputTextureUavIndex));

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kClientWidth / 8, WinApp::kClientHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());

	//NON_PIXEL_SHADER_RESOURCE >> RENDER_TARGET
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		inputResource_.Get());
}
