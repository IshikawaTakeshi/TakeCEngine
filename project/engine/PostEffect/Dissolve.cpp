#include "Dissolve.h"
#include "Utility/ResourceBarrier.h"
#include "base/TextureManager.h"
#include "ImGuiManager.h"
#include <cassert>

//=============================================================================
// 初期化
//=============================================================================
void Dissolve::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	//PostEffectの初期化
	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("DissolvePSO");
	//Bufferの名前付け
	inputResource_->SetName(L"Dissolve::inputResource_");
	outputResource_->SetName(L"Dissolve::outputResource_");

	//dissolveInfoResource生成
	dissolveInfoResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(DissolveInfo));
	dissolveInfoResource_->SetName(L"Dissolve::dissolveInfoResource_");
	dissolveInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&dissolveInfoData_));

	//dissolveInfo初期化
	dissolveInfoData_->threshold = 0.5f;
	dissolveInfoData_->isDissolve = false;

	//maskTexture読み込み
	maskTextureFilePath_ = "cloudNoise.png";
	TextureManager::GetInstance()->LoadTexture(maskTextureFilePath_,false);
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void Dissolve::UpdateImGui() {
#ifdef _DEBUG

	//Dissolve設定
	if(ImGui::TreeNode("Dissolve")) {
		ImGui::Text("Dissolve");
		ImGui::SliderFloat("DissolveThreshold", &dissolveInfoData_->threshold, 0.0f, 1.0f);
		ImGui::TreePop();
	}

	ImGui::SameLine();
	ImGui::Checkbox("##Dissolve::isActive", &dissolveInfoData_->isDissolve);


#endif // _DEBUG

}

//=============================================================================
// Dispatch
//=============================================================================
void Dissolve::Dispatch() {

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
	//maskTexture
	srvManager_->SetComputeRootDescriptorTable(
		computePSO_->GetComputeBindResourceIndex("gMaskTexture"),TextureManager::GetInstance()->GetSrvIndex(maskTextureFilePath_));
	//dissolveInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gDissolveInfo"), dissolveInfoResource_->GetGPUVirtualAddress());
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 2, WinApp::kScreenHeight / 2, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance()->Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());

}
