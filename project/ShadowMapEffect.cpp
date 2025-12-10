#include "ShadowMapEffect.h"
#include "engine/Utility/ResourceBarrier.h"
#include "engine/base/ImGuiManager.h"
#include "engine/camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include <cassert>

using namespace TakeC;

void ShadowMapEffect::Initialize(
	TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
	const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource,
	uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) {

	PostEffect::Initialize(dxCommon, srvManager, CSFilePath, inputResource, inputSrvIdx, outputResource);

	//PSOの名前を設定
	computePSO_->SetComputePipelineName("ShadowMap");

	//Bufferの名前を設定
	inputResource_->SetName(L"ShadowMap::InputResource");
	outputResource_->SetName(L"ShadowMap::_OutputResource");

	//depthテクスチャリソースを取得
	depthTextureSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforDepthTexture(dxCommon_->GetDepthStencilResource().Get(), depthTextureSrvIndex_);

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

void ShadowMapEffect::UpdateImGui() {


}

void ShadowMapEffect::Dispatch() {


}
