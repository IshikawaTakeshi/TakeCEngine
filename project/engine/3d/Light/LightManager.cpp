#include "LightManager.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "engine/base/PipelineStateObject.h"
#include "engine/base/ImGuiManager.h"
#include "engine/math/Vector3Math.h"

//=============================================================================
// 初期化
//=============================================================================
void LightManager::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	//DirectionalLightの初期化
	dirLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLightData));
	dirLightResource_->SetName(L"directionalLightResource_");
	dirLightData_ = nullptr;
	dirLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&dirLightData_));

	//PointLight用のResourceの作成
	pointLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(PointLightData) * kMaxPointLights);
	pointLightResource_->SetName(L"pointLightResource_");
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	std::memset(pointLightData_, 0, sizeof(PointLightData) * kMaxPointLights);
	activePointLightCount_ = 0;
	//SpotLight用のResourceの作成
	spotLightResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(SpotLightData) * kMaxSpotLights);
	spotLightResource_->SetName(L"spotLightResource_");
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	std::memset(spotLightData_, 0, sizeof(SpotLightData) * kMaxSpotLights);
	activeSpotLightCount_ = 0;

	//LightCount用のResourceの作成
	lightCountResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(LightCountData));
	lightCountResource_->SetName(L"Object3dCommon::lightCountResource_");
	lightCountData_ = nullptr;
	lightCountResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightCountData_));
	
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

	//DirectionalLight初期化
	//光源の色を書き込む
	dirLightData_->color_ = { 1.0f,1.0f,1.0f,1.0f };
	//光源の方向を書き込む
	dirLightData_->direction_ = { 0.0f,-0.05f,-0.95f };
	//光源の輝度書き込む
	dirLightData_->intensity_ = 1.0f;


	//ライト数データ初期化
	lightCountData_->pointLightCount = 0;
	lightCountData_->spotLightCount = 0;
}

//=============================================================================
// 終了・開放処理
//=============================================================================
void LightManager::Finalize() {
	dirLightResource_.Reset();
	pointLightResource_.Reset();
	spotLightResource_.Reset();
	lightCountResource_.Reset();
	dxCommon_ = nullptr;
	srvManager_ = nullptr;
	
}

//=============================================================================
// ポイントライト追加
//=============================================================================
uint32_t LightManager::AddPointLight(const PointLightData& lightData) {
	if (activePointLightCount_ >= kMaxPointLights) {
		return UINT32_MAX; // エラー
	}

	// マップされたメモリに直接書き込み
	pointLightData_[activePointLightCount_] = lightData;

	uint32_t index = activePointLightCount_;
	activePointLightCount_++;

	// ライト数を更新
	lightCountData_->pointLightCount = activePointLightCount_;

	return index;
}

//=============================================================================
// スポットライト追加
//=============================================================================

uint32_t LightManager::AddSpotLight(const SpotLightData& lightData) {
	
	if(activePointLightCount_>= kMaxSpotLights) {
		return UINT32_MAX; // エラー
	}

	// マップされたメモリに直接書き込み
	spotLightData_[activeSpotLightCount_] = lightData;

	uint32_t index = activeSpotLightCount_;
	activeSpotLightCount_++;

	// ライト数を更新
	lightCountData_->spotLightCount = activeSpotLightCount_;

	return index;
}

//=============================================================================
// ポイントライト削除
//=============================================================================
bool LightManager::RemovePointLight(uint32_t index) {
	if (index >= activePointLightCount_) {
		return false;
	}

	// 最後の要素を削除位置にコピー（スワップ削除）
	if (index < activePointLightCount_ - 1) {
		pointLightData_[index] = pointLightData_[activePointLightCount_ - 1];
	}

	// 最後の要素をゼロクリア
	std::memset(&pointLightData_[activePointLightCount_ - 1], 0, sizeof(PointLightData));

	activePointLightCount_--;
	lightCountData_->pointLightCount = activePointLightCount_;

	return true;
}

//=============================================================================
// スポットライト削除
//=============================================================================

bool LightManager::RemoveSpotLight(uint32_t index) {
	if (index >= activeSpotLightCount_) {
		return false;
	}
	// 最後の要素を削除位置にコピー（スワップ削除）
	if (index < activeSpotLightCount_ - 1) {
		spotLightData_[index] = spotLightData_[activeSpotLightCount_ - 1];
	}
	// 最後の要素をゼロクリア
	std::memset(&spotLightData_[activeSpotLightCount_ - 1], 0, sizeof(SpotLightData));
	activeSpotLightCount_--;
	lightCountData_->spotLightCount = activeSpotLightCount_;
	return true;
}

//=============================================================================
// ポイントライト更新
//=============================================================================
void LightManager::UpdatePointLight(uint32_t index, const PointLightData& light) {
	if (index >= activePointLightCount_) {
		return;
	}

	// マップされたメモリに直接書き込み
	pointLightData_[index] = light;
}

//=============================================================================
// スポットライト更新
//=============================================================================
void LightManager::UpdateSpotLight(uint32_t index, const SpotLightData& light) {
	if(index >= activeSpotLightCount_) {
		return;
	}
	// マップされたメモリに直接書き込み
	spotLightData_[index] = light;
}

//=============================================================================
// PointLightData取得
//=============================================================================

PointLightData* LightManager::GetPointLightData(uint32_t index) const {
	if (index >= activePointLightCount_) {
		return nullptr;
	}
	return &pointLightData_[index];
}

//=============================================================================
// SpotLightData取得
//=============================================================================
SpotLightData* LightManager::GetSpotLightData(uint32_t index) const {
	if (index >= activeSpotLightCount_) {
		return nullptr;
	}
	return &spotLightData_[index];
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

	//LightCount
	commandList->SetGraphicsRootConstantBufferView(
		pso->GetGraphicBindResourceIndex("gLightCount"),
		lightCountResource_->GetGPUVirtualAddress()
	);
}

//=============================================================================
// ImGui更新
//=============================================================================
void LightManager::UpdateImGui() {
#ifdef _DEBUG
	ImGui::Begin("LightManager");

	// DirectionalLight
	if (ImGui::TreeNode("DirectionalLight")) {
		ImGui::ColorEdit4("Color", &dirLightData_->color_.x);
		ImGui::DragFloat3("Direction", &dirLightData_->direction_.x, 0.01f);
		dirLightData_->direction_ = Vector3Math::Normalize(dirLightData_->direction_);
		ImGui::DragFloat("Intensity", &dirLightData_->intensity_, 0.01f, 0.0f, 10.0f);
		ImGui::TreePop();
	}

	// PointLights
	if (ImGui::TreeNode("PointLights")) {
		ImGui::Text("Active: %u / %u", activePointLightCount_, kMaxPointLights);

		for (uint32_t i = 0; i < activePointLightCount_; ++i) {
			std::string label = "Light " + std::to_string(i);
			if (ImGui::TreeNode(label.c_str())) {
				ImGui::ColorEdit4("Color", &pointLightData_[i].color_.x);
				ImGui::DragFloat3("Position", &pointLightData_[i].position_.x, 0.1f);
				ImGui::DragFloat("Intensity", &pointLightData_[i].intensity_, 0.01f, 0.0f, 10.0f);
				ImGui::DragFloat("Radius", &pointLightData_[i].radius_, 0.1f, 0.1f, 100.0f);
				ImGui::DragFloat("Decay", &pointLightData_[i].decay_, 0.01f, 0.0f, 10.0f);
				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Add Light") && activePointLightCount_ < kMaxPointLights) {
			PointLightData newLight{};
			newLight.color_ = {1.0f, 1.0f, 1.0f, 1.0f};
			newLight.position_ = {0.0f, 2.0f, 0.0f};
			newLight.intensity_ = 1.0f;
			newLight.radius_ = 10.0f;
			newLight.decay_ = 1.0f;
			AddPointLight(newLight);
		}

		ImGui::TreePop();
	}

	ImGui::End();
#endif
}