#include "PostEffect.h"
#include "Utility/ResourceBarrier.h"
#include <cassert>

PostEffect::~PostEffect() {
	computePSO_.reset();
	rootSignature_.Reset();
	inputResource_.Reset();
	outputResource_.Reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PostEffect::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager,const std::wstring& CSFilePath,ComPtr<ID3D12Resource> inputResource,uint32_t srvIndex) {

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
	inputTexSrvIndex_ = srvIndex;
	inputResource_ = inputResource;
	inputResource_->SetName(L"inputRenderTexture_");

	//outputRenderTexture
	outputTexUavIndex_ = srvManager_->Allocate();
	outputResource_ = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(),WinApp::kClientWidth,WinApp::kClientHeight,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	outputResource_->SetName(L"outputRenderTexture_");

	//UAVの生成
	srvManager_->CreateUAVforRenderTexture(outputResource_.Get(), outputTexUavIndex_);
}

void PostEffect::DisPatch() {

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

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kClientWidth / 8, WinApp::kClientHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());

	//NON_PIXEL_SHADER_RESOURCE >> PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		inputResource_.Get());
}