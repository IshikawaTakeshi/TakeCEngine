#include "ChromaticAberration.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include "engine/base/WinApp.h"
#include <cassert>
#include <algorithm>

using namespace TakeC;

//=============================================================================
// 初期化
//=============================================================================
void ChromaticAberration::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	// 親クラスの初期化処理
	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	// PSOの名前付け
	computePSO_->SetComputePipelineName("ChromaticAberrationPSO");
	// Bufferの名前付け
	inputResource_->SetName(L"ChromaticAberration::inputResource_");
	outputResource_->SetName(L"ChromaticAberration::outputResource_");

	// ChromaticAberrationInfoResource
	chromaticAberrationInfoResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ChromaticAberrationInfo));
	chromaticAberrationInfoResource_->SetName(L"ChromaticAberration::chromaticAberrationInfoResource_");
	
	// Mapping
	chromaticAberrationInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&chromaticAberrationInfoData_));
	chromaticAberrationInfoData_->center = { 0.5f, 0.5f }; // 中心
	chromaticAberrationInfoData_->intensity = 1.0f;        // 強度
	chromaticAberrationInfoData_->redScale = 1.01f;        // 赤の拡大率
	chromaticAberrationInfoData_->greenScale = 1.00f;      // 緑の拡大率
	chromaticAberrationInfoData_->blueScale = 0.99f;       // 青の拡大率
	chromaticAberrationInfoData_->isActive = 0;
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void ChromaticAberration::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)
	if (ImGui::TreeNode("ChromaticAberration")) {
		bool bActive = chromaticAberrationInfoData_->isActive != 0;
		if (ImGui::Checkbox("isActive", &bActive)) {
			chromaticAberrationInfoData_->isActive = bActive ? 1 : 0;
		}
		
		ImGui::SliderFloat2("Center", &chromaticAberrationInfoData_->center.x, 0.0f, 1.0f);
		ImGui::SliderFloat("Intensity", &chromaticAberrationInfoData_->intensity, 0.0f, 2.0f);
		ImGui::SliderFloat("RedScale", &chromaticAberrationInfoData_->redScale, 0.9f, 1.1f, "%.5f");
		ImGui::SliderFloat("GreenScale", &chromaticAberrationInfoData_->greenScale, 0.9f, 1.1f, "%.5f");
		ImGui::SliderFloat("BlueScale", &chromaticAberrationInfoData_->blueScale, 0.9f, 1.1f, "%.5f");
		ImGui::TreePop();
	}
#endif // _DEBUG
}

//=============================================================================
// Dispatch
//=============================================================================
void ChromaticAberration::Dispatch() {

	// NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	// Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	
	// inputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	// outputTex
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);
	// ChromaticAberrationInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(computePSO_->GetComputeBindResourceIndex("gChromaInfo"), chromaticAberrationInfoResource_->GetGPUVirtualAddress());
	
	// Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	// UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}

void ChromaticAberration::SetIntensity(float intensity) {
	chromaticAberrationInfoData_->intensity = std::max(0.0f, intensity);
}
