#include "ShadowMapEffect.h"
#include "engine/Utility/ResourceBarrier.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include <cassert>

using namespace TakeC;

//=========================================================
// 初期化
//=========================================================
void ShadowMapEffect::Initialize(
	TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource,
	uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);

	//PSOの名前を設定
	computePSO_->SetComputePipelineName("ShadowMapPSO");

	//Bufferの名前を設定
	inputResource_->SetName(L"ShadowMap::InputResource");
	outputResource_->SetName(L"ShadowMap::_OutputResource");

	//シャドウマップエフェクト情報用バッファ生成
	shadowMapEffectInfoResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ShadowMapEffectInfo));
	shadowMapEffectInfoResource_->SetName(L"ShadowMap::shadowMapEffectInfoResource_");
	shadowMapEffectInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&shadowMapEffectInfo_));

	//初期値設定
	shadowMapEffectInfo_->isActive = true;
	shadowMapEffectInfo_->bias = 0.005f;
	shadowMapEffectInfo_->normalBias = 0.05f;
	shadowMapEffectInfo_->pcfRange = 1.0f;
}

//=========================================================
// ImGuiの更新
//=========================================================
void ShadowMapEffect::UpdateImGui() {

	if(ImGui::TreeNode("ShadowMapEffect")){
		ImGui::Checkbox("Enable", &shadowMapEffectInfo_->isActive);
		ImGui::SliderFloat("Bias", &shadowMapEffectInfo_->bias, 0.0f, 0.05f, "%.5f");
		ImGui::SliderFloat("NormalBias", &shadowMapEffectInfo_->normalBias, 0.0f, 0.2f, "%.5f");
		ImGui::SliderFloat("PCFRange", &shadowMapEffectInfo_->pcfRange, 0.0f, 5.0f, "%.5f");
		ImGui::TreePop();
	}
}

//=========================================================
// 更新処理
//=========================================================
void ShadowMapEffect::Dispatch() {


	//DEPTH_WRITE >> NON_PIXEL_SHADER_RESOURCE
	/*ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		lightCameraDepthTextureResource_.Get());*/

	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		depthTextureResource_.Get());

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
	//slightCameraDepth
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gShadowMap"), lightCameraDepthTextureSrvIndex_);
	//sceneDepth
	srvManager_->SetComputeRootDescriptorTable(computePSO_->GetComputeBindResourceIndex("gSceneDepth"), depthTextureSrvIndex_);

	//shadowMapEffectInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gShadowMapEffectInfo"),
		shadowMapEffectInfoResource_->GetGPUVirtualAddress()
	);

	//lightCameraInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gLightCameraInfo"),
		TakeCFrameWork::GetLightManager()->GetLightCameraInfoResource()->GetGPUVirtualAddress()
	);

	//gMainCameraInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gMainCameraInfo"),
	TakeC::CameraManager::GetInstance().GetActiveCamera()->GetCameraResource()->GetGPUVirtualAddress()
	);
	
	//Dispatch
	dxCommon_->GetCommandList()->Dispatch(TakeC::WinApp::kScreenWidth / 8, TakeC::WinApp::kScreenHeight / 8, 1);
	//UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());

	//NON_PIXEL_SHADER_RESOURCE >> DEPTH_WRITE
	/*ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		lightCameraDepthTextureResource_.Get());*/

	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		depthTextureResource_.Get());
}