#include "ScanlineEffect.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Utility/ResourceBarrier.h"
#include "engine/Base/TakeCFrameWork.h"

//=============================================================================
// 初期化
//=============================================================================
void ScanlineEffect::Initialize(
	TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource,
	uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	//共通の初期化処理
	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	//エフェクトパラメータ用のリソース生成
	computePSO_->SetComputePipelineName("ScanlineEffectPSO");

	//Bufferの名前付け
	inputResource_->SetName(L"ScanlineEffect::inputResource_");
	outputResource_->SetName(L"ScanlineEffect::outputResource_");

	//エフェクトパラメータ用のリソース生成
	CreateParamResource();
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void ScanlineEffect::UpdateImGui() {

	if (ImGui::TreeNode("ScanlineEffect")) {
		ImGui::SliderFloat("Intensity", &param_->intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Frequency", &param_->frequency, 1.0f, 1000.0f);
		ImGui::SliderFloat("Speed", &param_->speed, 0.0f, 10.0f);
		ImGui::SliderFloat("Thickness", &param_->thickness, 0.0f, 1.0f);
		ImGui::TreePop();
	}
	ImGui::SameLine();
	ImGui::Checkbox("##ScanlineEffect::isActive", &param_->isActive);
}

//=============================================================================
// ディスパッチ
//=============================================================================
void ScanlineEffect::Dispatch() {

	param_->time += TakeCFrameWork::GetDeltaTime();

	//NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	//Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	//inputTexure
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	//outputTexure
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);

	//ScanlineParam
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gScanlineParam"),
		paramResource_->GetGPUVirtualAddress());

	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(TakeC::WinApp::kScreenWidth / 8, TakeC::WinApp::kScreenHeight / 8, 1);

	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}


//=============================================================================
// エフェクトパラメータ用のリソースを生成する
//=============================================================================
void ScanlineEffect::CreateParamResource() {

	paramResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ScanlineParam));
	paramResource_->SetName(L"ScanlineEffect::paramResource_");
	paramResource_->Map(0, nullptr, reinterpret_cast<void**>(&param_));
	param_->intensity = 0.35f;
	param_->frequency = 420.0f;
	param_->speed = 2.0f;
	param_->thickness = 0.35f;
}
