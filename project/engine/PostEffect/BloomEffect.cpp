#include "BloomEffect.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <cassert>


//=============================================================================
// 初期化
//=============================================================================
void BloomEffect::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	dxCommon_ = dxCommon;
	PostEffect::Initialize(dxCommon_, srvManager, CSFilePath, inputResource, inputSrvIdx,outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("BloomEffectPSO");
	//Bufferの名前付け
	inputResource_->SetName(L"Bloom::inputResource_");
	outputResource_->SetName(L"Bloom::outputResource_");

	// エフェクト情報用バッファ生成
	effectInfoResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(BloomEffectInfo));
	effectInfoResource_->SetName(L"Bloom::effectInfoResource_");
	effectInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&effectInfoData_));

	// 解像度取得
	D3D12_RESOURCE_DESC desc = inputResource->GetDesc();
	uint32_t fullWidth = static_cast<uint32_t>(desc. Width);
	uint32_t fullHeight = static_cast<uint32_t>(desc.Height);
	halfWidth_ = fullWidth / 2;
	halfHeight_ = fullHeight / 2;

	//UAVリソース生成
	brightPassResource_ = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(),
		halfWidth_, halfHeight_,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	brightPassResource_->SetName(L"Bloom::brightPassResource_");
	horizontalBlurResource_ = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(),
		halfWidth_, halfHeight_,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	horizontalBlurResource_->SetName(L"Bloom::horizontalBlurResource_");
	verticalBlurResource_ = dxCommon_->CreateTextureResourceUAV(
		dxCommon_->GetDevice(),
		halfWidth_, halfHeight_,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	verticalBlurResource_->SetName(L"Bloom::verticalBlurResource_");

	//各PSO生成
	brightPassPSO_ = std::make_unique<PSO>();
	brightPassPSO_->CompileComputeShader(dxCommon_->GetDXC(), L"PostEffect/BloomBrightPass.CS.hlsl");
	horizontalBlurPSO_ = std::make_unique<PSO>();
	horizontalBlurPSO_->CompileComputeShader(dxCommon_->GetDXC(), L"PostEffect/BloomBlurHorizontal.CS.hlsl");
	verticalBlurPSO_ = std::make_unique<PSO>();
	verticalBlurPSO_->CompileComputeShader(dxCommon_->GetDXC(), L"PostEffect/BloomBlurVertical.CS.hlsl");

	brightPassPSO_->CreateComputePSO(dxCommon_->GetDevice());
	horizontalBlurPSO_->CreateComputePSO(dxCommon_->GetDevice());
	verticalBlurPSO_->CreateComputePSO(dxCommon_->GetDevice());
	rootSignatureForBloomTexture_ = brightPassPSO_->GetComputeRootSignature();

	//UAVインデックス取得/UAV生成
	brightPassUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforRenderTexture(
		brightPassResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		brightPassUavIndex_);
	horizontalBlurUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforRenderTexture(
		horizontalBlurResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		horizontalBlurUavIndex_);
	verticalBlurUavIndex_ = srvManager_->Allocate();
	srvManager_->CreateUAVforRenderTexture(
		verticalBlurResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		verticalBlurUavIndex_);
	//SRVインデックス取得/SRV生成
	brightPassSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(
		brightPassResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		brightPassSrvIndex_);
	horizontalBlurSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(
		horizontalBlurResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		horizontalBlurSrvIndex_);
	verticalBlurSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforRenderTexture(
		verticalBlurResource_.Get(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		verticalBlurSrvIndex_);

	//エフェクト情報初期化
	effectInfoData_->isActive = true;
	effectInfoData_->threshold = 0.75f;
	effectInfoData_->strength = 8.0f;
	effectInfoData_->sigma = 2.0f;
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void BloomEffect::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Text("BloomEffect");
	
	if (ImGui::TreeNode("BloomEffect")) {
		ImGui::SliderFloat("threshold", &effectInfoData_->threshold, 0.0f, 1.0f);
		ImGui::SliderFloat("strength", &effectInfoData_->strength, 0.0f, 100.0f);
		ImGui::SliderFloat("sigma", &effectInfoData_->sigma, 0.0f, 20.0f);
		ImGui::TreePop();
	}
	ImGui::SameLine();
	ImGui::Checkbox("isActive", &effectInfoData_->isActive);
#endif // _DEBUG

}

//=============================================================================
// Dispatch
//=============================================================================
void BloomEffect::Dispatch() {

	// Pass1: 輝度抽出
	// 入力: inputResource_ (元画像)
	// 出力: brightPassResource_
	DispatchBrightPass();

	// Pass2: 水平ブラー
	// 入力: brightPassResource_
	// 出力: verticalBlurResource_
	DispatchHorizontalBlur();

	// Pass3: 垂直ブラー
	// 入力: blurTempResource_
	// 出力: bloomResource_ (= gBloomTexture)
	DispatchVerticalBlur();

	// Pass4: 合成
	// 入力1: inputResource_ (元画像 = gOriginalTexture)
	// 入力2: bloomResource_ (ブラー結果 = gBloomTexture)
	// 出力: outputResource_
	DispatchComposite();
}

//=============================================================================
// 輝度抽出ディスパッチ
//=============================================================================
void BloomEffect::DispatchBrightPass() {

	//Computeパイプラインのセット
	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		brightPassResource_.Get());
	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureForBloomTexture_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(brightPassPSO_->GetComputePipelineState());
	//inputTex
	srvManager_->SetComputeRootDescriptorTable(brightPassPSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	//outputTex
	srvManager_->SetComputeRootDescriptorTable(brightPassPSO_->GetComputeBindResourceIndex("gOutputTexture"), brightPassUavIndex_);
	//effectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		brightPassPSO_->GetComputeBindResourceIndex("gBloomEffectInfo"), effectInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 16, WinApp::kScreenHeight / 16, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		brightPassResource_.Get());
}

//=============================================================================
// 水平ブラーディスパッチ
//=============================================================================
void BloomEffect::DispatchHorizontalBlur() {

	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		horizontalBlurResource_.Get());
	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureForBloomTexture_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(horizontalBlurPSO_->GetComputePipelineState());
	//inputTex
	srvManager_->SetComputeRootDescriptorTable(horizontalBlurPSO_->GetComputeBindResourceIndex("gInputTexture"), brightPassSrvIndex_);
	//outputTex
	srvManager_->SetComputeRootDescriptorTable(horizontalBlurPSO_->GetComputeBindResourceIndex("gOutputTexture"),horizontalBlurUavIndex_);
	//effectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		horizontalBlurPSO_->GetComputeBindResourceIndex("gBloomEffectInfo"), effectInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch((halfWidth_ + 255) / 256, halfHeight_, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		horizontalBlurResource_.Get());
}

//=============================================================================
// 垂直ブラーディスパッチ
//=============================================================================
void BloomEffect::DispatchVerticalBlur() {

	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		verticalBlurResource_.Get());
	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignatureForBloomTexture_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(verticalBlurPSO_->GetComputePipelineState());
	//inputTex
	srvManager_->SetComputeRootDescriptorTable(verticalBlurPSO_->GetComputeBindResourceIndex("gInputTexture"), horizontalBlurSrvIndex_);
	//outputTex
	srvManager_->SetComputeRootDescriptorTable(verticalBlurPSO_->GetComputeBindResourceIndex("gOutputTexture"), verticalBlurUavIndex_);
	//effectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		verticalBlurPSO_->GetComputeBindResourceIndex("gBloomEffectInfo"), effectInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(halfWidth_, (halfHeight_ + 255) / 256, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		verticalBlurResource_.Get());
}

//=============================================================================
// 合成ディスパッチ
//=============================================================================
void BloomEffect::DispatchComposite() {

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
	//effectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gBloomEffectInfo"), effectInfoResource_->GetGPUVirtualAddress());
	//gBloomTexture
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gBloomTexture"), verticalBlurSrvIndex_);
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 16, WinApp::kScreenHeight / 16, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}