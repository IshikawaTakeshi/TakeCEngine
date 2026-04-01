#include "DistortionEffect.h"
#include "Utility/ResourceBarrier.h"
#include "base/TextureManager.h"
#include "engine/Base/ImGuiManager.h"
#include "engine/Base/TakeCFrameWork.h"
#include <cassert>

using namespace TakeC;

//=============================================================================
// 初期化
//=============================================================================
void DistortionEffect::Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
	ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	// 基底クラスの初期化
	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);
	//PSOの名前付け
	computePSO_->SetComputePipelineName("DistortionEffectPSO");
	//Bufferの名前付け
	inputResource_->SetName(L"DistortionEffect::inputResource_");
	outputResource_->SetName(L"DistortionEffect::outputResource_");

	// 定数バッファ生成
	distortionInfoResource_ = dxCommon_->CreateBufferResource(dxCommon->GetDevice(), sizeof(DistortionInfo));
	distortionInfoResource_->SetName(L"DistortionEffect::distortionInfoResource_");
	// Mapping
	distortionInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&distortionInfo_));

	// パラメータ初期値
	distortionInfo_->strength = 0.01f;
	distortionInfo_->timer = 0.0f;
	distortionInfo_->scrollSpeed = Vector2(0.1f, 0.1f);
	distortionInfo_->noiseScale = Vector2(1.0f, 1.0f);
	distortionInfo_->offset = Vector2(0.5f, 0.5f); // デフォルトでは0.5（UNORMテクスチャの中央）を引く
	distortionInfo_->enable = false;

	// ノイズテクスチャ読み込み
	noiseTextureFilePath_ = "cloudNoise.png";
	TextureManager::GetInstance().LoadTexture(noiseTextureFilePath_, false);
}

//=============================================================================
// ImGuiの更新
//=============================================================================
void DistortionEffect::UpdateImGui() {
#if defined(_DEBUG) || defined(_DEVELOP)

	if (ImGui::TreeNode("DistortionEffect")) {
		ImGui::SliderFloat("Strength", &distortionInfo_->strength, 0.0f, 0.5f, "%.5f");
		ImGui::SliderFloat2("ScrollSpeed", &distortionInfo_->scrollSpeed.x, -100.0f, 1.000f, "%.3f");
		ImGui::SliderFloat2("NoiseScale", &distortionInfo_->noiseScale.x, 0.01f, 1000.0f, "%.2f");
		ImGui::SliderFloat2("Offset (Center)", &distortionInfo_->offset.x, 0.0f, 1.0f, "%.3f");
		ImGui::TreePop();
	}

	ImGui::SameLine();
	ImGui::Checkbox("##DistortionEffect::isActive", &distortionInfo_->enable);

#endif // _DEBUG
}

//=============================================================================
// Dispatch
//=============================================================================
void DistortionEffect::Dispatch() {

	// 経過時間を加算
	distortionInfo_->timer += TakeCFrameWork::GetDeltaTime();

	// NON_PIXEL_SHADER_RESOURCE >> UNORDERED_ACCESS
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		outputResource_.Get());

	// Computeパイプラインのセット
	dxCommon_->GetCommandList()->SetComputeRootSignature(rootSignature_.Get());
	dxCommon_->GetCommandList()->SetPipelineState(computePSO_->GetComputePipelineState());
	// inputTex
	srvManager_->SetComputeRootDescriptorTable(
		computePSO_->GetComputeBindResourceIndex("gInputTexture"), inputTexSrvIndex_);
	// outputTex
	srvManager_->SetComputeRootDescriptorTable(
		computePSO_->GetComputeBindResourceIndex("gOutputTexture"), outputTexUavIndex_);
	// noiseTexture
	srvManager_->SetComputeRootDescriptorTable(
		computePSO_->GetComputeBindResourceIndex("gNoiseTexture"), TextureManager::GetInstance().GetSrvIndex(noiseTextureFilePath_));
	// distortionInfo
	dxCommon_->GetCommandList()->SetComputeRootConstantBufferView(
		computePSO_->GetComputeBindResourceIndex("gDistortionInfo"), distortionInfoResource_->GetGPUVirtualAddress());
	// Dispatch
	dxCommon_->GetCommandList()->Dispatch(WinApp::kScreenWidth / 8, WinApp::kScreenHeight / 8, 1);

	// UNORDERED_ACCESS >> NON_PIXEL_SHADER_RESOURCE
	ResourceBarrier::GetInstance().Transition(
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		outputResource_.Get());
}

void DistortionEffect::ApplySpecificParams(const nlohmann::json& params) {

	// パラメータが存在しない場合は何もしない
	if (params.is_null() || params.empty()) {
		return;
	}

	auto param = params.get<DistortionInfo>();
	distortionInfo_->strength = param.strength;
	distortionInfo_->scrollSpeed = param.scrollSpeed;
	distortionInfo_->noiseScale = param.noiseScale;
	distortionInfo_->offset = param.offset;
	distortionInfo_->enable = param.enable;
}

nlohmann::json DistortionEffect::GetSpecificParams() const {
	
	DistortionInfo param{};
	param.strength = distortionInfo_->strength;
	param.scrollSpeed = distortionInfo_->scrollSpeed;
	param.noiseScale = distortionInfo_->noiseScale;
	param.offset = distortionInfo_->offset;
	param.enable = distortionInfo_->enable;

	return param;
}

//=============================================================================
// 強度の設定
//=============================================================================
void DistortionEffect::SetIntensity(float intensity) {
	distortionInfo_->strength = intensity;
}
