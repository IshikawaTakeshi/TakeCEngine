#include "PostEffect.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>

PostEffect::~PostEffect() {
	computePSO_.reset();
	rootSignature_.Reset();
	inputResource_.Reset();
	outputResource_.Reset();
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
}

void PostEffect::Initialize(
	DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,uint32_t inputUavIndex,ComPtr<ID3D12Resource> outputResource) {

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
	inputTexSrvIndex_ = inputSrvIdx;
	inputTexUavIndex_ = inputUavIndex;
	inputResource_ = inputResource;
	inputResource_->SetName(L"inputRenderTexture_");

	//srv生成
	srvManager_->CreateSRVforRenderTexture(inputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM, inputTexSrvIndex_);
	//uav生成
	//srvManager_->CreateUAVforRenderTexture(inputResource_.Get(), inputTexUavIndex_);

	//outputRenderTexture
	outputTexSrvIndex_ = srvManager_->Allocate();
	outputTexUavIndex_ = srvManager_->Allocate();
	outputResource_ = outputResource;
	outputResource_->SetName(L"outputRenderTexture_");

	//srv生成
	srvManager_->CreateSRVforRenderTexture(outputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, outputTexSrvIndex_);
	//uav生成
	srvManager_->CreateUAVforRenderTexture(outputResource_.Get(),DXGI_FORMAT_R8G8B8A8_UNORM, outputTexUavIndex_);
}

void PostEffect::UpdateImGui() {
	
}

void PostEffect::DisPatch() {

	////NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	//ResourceBarrier::GetInstance()->Transition(
	//	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	//	outputResource_.Get());

	////Computeパイプラインのセット
	//dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	//dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	////inputTex
	//srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	////outputTex
	//srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);

	////Dispatch
	//dxCommon_->GetCommandList()->Dispatch(WinApp::kClientWidth / 8, WinApp::kClientHeight / 8, 1);

	////UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	//ResourceBarrier::GetInstance()->Transition(
	//	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	//	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//	outputResource_.Get());

	////NON_PIXEL_SHADER_RESOURCE >> PIXEL_SHADER_RESOURCE
	//ResourceBarrier::GetInstance()->Transition(
	//	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//	inputResource_.Get());
}