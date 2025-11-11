#include "LightManager.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "engine/base/PipelineStateObject.h"

//=============================================================================
// 初期化
//=============================================================================
void LightManager::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//DirectionalLightの初期化
	dirLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLightData));
	dirLightResource_->SetName(L"Object3dCommon::directionalLightResource_");
	dirLightData_ = nullptr;
	//データを書き込むためのアドレスを取得
	dirLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&dirLightData_));
	//光源の色を書き込む
	dirLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	dirLightData_->direction_ = { 0.0f,-0.05f,-0.95f };
	//光源の輝度書き込む
	dirLightData_->intensity_ = 1.0f;

	//PointLight用のResourceの作成
	pointLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLightData) * kMaxPointLights);
	pointLightResource_->SetName(L"Object3dCommon::pointLightResource_");
	//SpotLight用のResourceの作成
	spotLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLightData) * kMaxSpotLights);
	spotLightResource_->SetName(L"Object3dCommon::spotLightResource_");
	
	//SRVの生成(PointLight)
	pointLightSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(
		kMaxPointLights,
		sizeof(PointLightData),
		pointLightResource_.Get(),
		pointLightSrvIndex_
	);
	//SRVの生成(SpotLight)
	spotLightSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(
		kMaxSpotLights,
		sizeof(SpotLightData),
		spotLightResource_.Get(),
		spotLightSrvIndex_
	);

	//ライトデータ初期化
	pointLightData_.clear();
	spotLightData_.clear();
}

//=============================================================================
// ポイントライト追加
//=============================================================================
uint32_t LightManager::AddPointLight(const PointLightData& lightData) {
	if (pointLightData_.size() >= kMaxPointLights) {
		return UINT32_MAX; // エラー
	}

	pointLightData_.push_back(lightData);
	return static_cast<uint32_t>(pointLightData_.size() - 1);
}

//=============================================================================
// スポットライト追加
//=============================================================================

uint32_t LightManager::AddSpotLight(const SpotLightData& lightData) {
	if (spotLightData_.size() >= kMaxSpotLights) {
		return UINT32_MAX; // エラー
	}
	spotLightData_.push_back(lightData);
	return static_cast<uint32_t>(spotLightData_.size() - 1);
}

//=============================================================================
// ポイントライト更新
//=============================================================================
void LightManager::UpdatePointLight(uint32_t index, const PointLightData& light) {
	//範囲外チェック
	if(index >= pointLightData_.size()) {
		return;
	}

	pointLightData_[index] = light;
}

//=============================================================================
// スポットライト更新
//=============================================================================
void LightManager::UpdateSpotLight(uint32_t index, const SpotLightData& light) {
	//範囲外チェック
	if (index >= spotLightData_.size()) {
		return;
	}
	spotLightData_[index] = light;
}

//=============================================================================
// GPUへデータ転送
//=============================================================================
void LightManager::TransferToGPU() {

	//DirLightは初期化時に転送済み

	//PointLightデータ転送
	if (!pointLightData_.empty()) {
		PointLightData* mappedData = nullptr;
		pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));

		std::span<const PointLightData> lightsSpan = GetPointLights();
		std::memcpy(mappedData, lightsSpan.data(), 
			sizeof(PointLightData) * lightsSpan.size());

		pointLightResource_->Unmap(0, nullptr);
	}
	//SpotLightデータ転送
	if (!spotLightData_.empty()) {
		SpotLightData* mappedData = nullptr;
		spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
		std::span<const SpotLightData> lightsSpan = GetSpotLights();
		std::memcpy(mappedData, lightsSpan.data(),
			sizeof(SpotLightData) * lightsSpan.size());
		spotLightResource_->Unmap(0, nullptr);
	}
}

//=============================================================================
// ライト用リソースの設定
//=============================================================================

void LightManager::SetLightResources(PSO* pso) {

	//コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//DirectionalLight
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gDirectionalLight"),
		dirLightResource_->GetGPUVirtualAddress()
	);
	//PointLight
	srvManager_->SetGraphicsRootDescriptorTable(pso->GetGraphicBindResourceIndex("gPointLight"),pointLightSrvIndex_);
	//SpotLight
	srvManager_->SetGraphicsRootDescriptorTable(pso->GetGraphicBindResourceIndex("gSpotLight"), spotLightSrvIndex_);
}
