#include "LightManager.h"

#include <numbers>
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "engine/base/PipelineStateObject.h"
#include "engine/base/ImGuiManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Camera/Camera.h"

//=============================================================================
// 初期化
//=============================================================================
void TakeC::LightManager::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {

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

	//LightCameraInfo用のResourceの作成
	lightCameraInfoResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(LightCameraInfo));
	lightCameraInfoResource_->SetName(L"LightManager::lightCameraInfoResource_");
	lightCameraInfo_ = nullptr;
	lightCameraInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightCameraInfo_));
	
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
void TakeC::LightManager::Finalize() {
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
uint32_t TakeC::LightManager::AddPointLight(const PointLightData& lightData) {
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

uint32_t TakeC::LightManager::AddSpotLight(const SpotLightData& lightData) {
	
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
bool TakeC::LightManager::RemovePointLight(uint32_t index) {
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

bool TakeC::LightManager::RemoveSpotLight(uint32_t index) {
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
void TakeC::LightManager::UpdatePointLight(uint32_t index, const PointLightData& light) {
	if (index >= activePointLightCount_) {
		return;
	}

	// マップされたメモリに直接書き込み
	pointLightData_[index] = light;
}

//=============================================================================
// スポットライト更新
//=============================================================================
void TakeC::LightManager::UpdateSpotLight(uint32_t index, const SpotLightData& light) {
	if(index >= activeSpotLightCount_) {
		return;
	}
	// マップされたメモリに直接書き込み
	spotLightData_[index] = light;
}

//=============================================================================
// シャドウマトリックス更新
//=============================================================================
void TakeC::LightManager::UpdateShadowMatrix(Camera* camera) {
	lightCamera_ = camera;

	// directionalLightをカメラの方向に向ける（これは問題ないので維持）
	dirLightData_->direction_ = Vector3Math::Normalize(lightCamera_->GetDirection());

	// --- 修正箇所：ライトのカメラの中心点を決定 ---

	// 1. シーンの中心位置（＝シャドウマップを合わせたい場所）を取得
	// プレイヤーの位置や、メインカメラの注視点（ターゲット）を利用する
	// 仮に、渡されたカメラが持つターゲット位置をシーンの中心とします。
	Vector3 sceneCenter = camera->GetTargetPosition(); // ※ 適切なメソッド名に置き換えてください

	// 2. ライトの方向と距離に基づいて、ライトカメラの位置を決定
	Vector3 lightDir = dirLightData_->direction_;
	float distance = lightCameraDistance_; // LightManager.h で定義: 100.0f

	// ライトカメラの位置: 中心からライトの逆方向に distance 離れた位置
	Vector3 lightPos = sceneCenter - lightDir * distance;

	// 3. ライトのビュー行列の計算
	// lightPos から sceneCenter を見ているビュー行列
	Matrix4x4 lightView = MatrixMath::LookAt(
		lightPos,
		sceneCenter,
		Vector3(0.0f, 1.0f, 0.0f) // アップベクトル
	);

	// --- 修正箇所：正射影範囲に shadowRange を使用 ---

	// 4. ライト射影行列の計算
	// LightManager.h で定義された shadowRange を利用
	float halfRange = shadowRange_ * 0.5f;

	Matrix4x4 lightProj = MatrixMath::MakeOrthographicMatrix(
		-halfRange,        // Left
		halfRange,         // Top
		halfRange,         // Right
		-halfRange,        // Bottom
		nearClip_, farClip_    // Near, Far (farClip も LightManager.hで定義)
	);

	lightCameraInfo_->viewProjection_ = lightView * lightProj;
	lightCameraInfo_->position_ = lightPos;
}

//=============================================================================
// ポイントライト描画
//=============================================================================
void TakeC::LightManager::DrawPointLights() {

	// アクティブなポイントライトが無ければ終了
	if(activePointLightCount_ == 0) {
		return;
	}

	// 全てのポイントライト一覧を表示
	for (uint32_t i = 0; i < activePointLightCount_; ++i) {

		TakeCFrameWork::GetWireFrame()->DrawPointLight(
			pointLightData_[i].position_,
			Vector3{0.0f,0.0f,-1.0f},
			pointLightData_[i].radius_,
			pointLightData_[i].color_
		);
	}
}

//=============================================================================
// スポットライト描画
//=============================================================================
void TakeC::LightManager::DrawSpotLights() {

	// アクティブなスポットライトが無ければ終了
	if (activeSpotLightCount_ == 0) {
		return;
	}
	// 全てのスポットライト一覧を表示
	for (uint32_t i = 0; i < activeSpotLightCount_; ++i) {
		TakeCFrameWork::GetWireFrame()->DrawCone(
			spotLightData_[i].position_,
			spotLightData_[i].direction_,
			std::acos(spotLightData_[i].cosAngle_),
			spotLightData_[i].distance_,
			spotLightData_[i].color_
		);
	}
}

//=============================================================================
// PointLightData取得
//=============================================================================

PointLightData* TakeC::LightManager::GetPointLightData(uint32_t index) const {
	if (index >= activePointLightCount_) {
		return nullptr;
	}
	return &pointLightData_[index];
}

//=============================================================================
// SpotLightData取得
//=============================================================================
SpotLightData* TakeC::LightManager::GetSpotLightData(uint32_t index) const {
	if (index >= activeSpotLightCount_) {
		return nullptr;
	}
	return &spotLightData_[index];
}

//=============================================================================
// ライト用リソースの設定
//=============================================================================

void TakeC::LightManager::SetLightResources(PSO* pso) {

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
void TakeC::LightManager::UpdateImGui() {
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

		// 全てのポイントライト一覧を表示
		for (uint32_t i = 0; i < activePointLightCount_; ++i) {
			std::string label = "Light " + std::to_string(i);
			std::string colorButtonId = "##PointLightColor" + std::to_string(i);

			bool nodeOpen = ImGui::TreeNode(label. c_str());
			ImGui::SameLine();
			ImGui::ColorButton(
				colorButtonId. c_str(),
				ImVec4(
					pointLightData_[i].color_.x,
					pointLightData_[i].color_.y,
					pointLightData_[i].color_.z,
					pointLightData_[i].color_. w),
				ImGuiColorEditFlags_NoTooltip, ImVec2(16, 16));

			if (nodeOpen) {
				ImGui::ColorEdit4("Color", &pointLightData_[i].color_.x);
				ImGui::DragFloat3("Position", &pointLightData_[i].position_.x, 0.1f);
				ImGui::DragFloat("Intensity", &pointLightData_[i].intensity_, 0.01f, 0.0f, 100.0f);
				ImGui::DragFloat("Radius", &pointLightData_[i]. radius_, 0.1f, 0.1f, 100.0f);
				ImGui::DragFloat("Decay", &pointLightData_[i]. decay_, 0.01f, 0.0f, 10.0f);
				ImGui::TreePop();
			}
		}

		// ポイントライト追加ボタン
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

	// SpotLights
	if (ImGui::TreeNode("SpotLights")) {
		ImGui::Text("Active: %u / %u", activeSpotLightCount_, kMaxSpotLights);
		// 全てのスポットライト一覧を表示
		for (uint32_t i = 0; i < activeSpotLightCount_; ++i) {
			std::string label = "Light " + std::to_string(i);
			std::string colorButtonId = "##SpotLightColor" + std::to_string(i);

			bool nodeOpen = ImGui::TreeNode(label. c_str());
			ImGui::SameLine();
			ImGui::ColorButton(colorButtonId. c_str(), ImVec4(spotLightData_[i].color_.x, spotLightData_[i].color_.y, spotLightData_[i].color_.z, spotLightData_[i].color_. w), ImGuiColorEditFlags_NoTooltip, ImVec2(16, 16));

			if (nodeOpen) {
				ImGui::ColorEdit4("Color", &spotLightData_[i].color_.x);
				ImGui::DragFloat3("Position", &spotLightData_[i].position_.x, 0.1f);
				ImGui::DragFloat3("Direction", &spotLightData_[i].direction_.x, 0.01f);
				spotLightData_[i].direction_ = Vector3Math::Normalize(spotLightData_[i]. direction_);
				ImGui::DragFloat("Distance", &spotLightData_[i].distance_, 0.1f, 0.1f, 100.0f);
				ImGui::DragFloat("Intensity", &spotLightData_[i].intensity_, 0.01f, 0.0f, 100.0f);
				ImGui::DragFloat("Decay", &spotLightData_[i]. decay_, 0.01f, 0.0f, 10.0f);
				ImGui::DragFloat("CosAngle", &spotLightData_[i].cosAngle_, 0.001f, 0.0f, 1.0f);
				ImGui::DragFloat("PenumbraAngle", &spotLightData_[i]. penumbraAngle_, 0.001f, 0.0f, 1.0f);
				ImGui::TreePop();
			}
		}
		// スポットライト追加ボタン
		if (ImGui::Button("Add Light") && activeSpotLightCount_ < kMaxSpotLights) {
			SpotLightData newLight{};
			newLight.color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
			newLight.position_ = { 2.0f, 2.0f, 0.0f };
			newLight.direction_ = { -1.0f, -1.0f, 0.0f };
			newLight.distance_ = 10.0f;
			newLight.intensity_ = 1.0f;
			newLight.decay_ = 1.0f;
			newLight.cosAngle_ = std::cosf(std::numbers::pi_v<float> / 4.0f);
			newLight.penumbraAngle_ = std::numbers::pi_v<float> / 8.0f;
			AddSpotLight(newLight);
		}
		ImGui::TreePop();
	}

	ImGui::Separator();
	ImGui::DragFloat("Light Camera Distance", &lightCameraDistance_, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("Shadow Range", &shadowRange_, 1.0f, 1.0f, 10000.0f);
	ImGui::DragFloat("Far Clip", &farClip_, 1.0f, 10.0f, 10000.0f);
	ImGui::DragFloat("Near Clip", &nearClip_, 0.1f, 0.1f, 1000.0f);

	ImGui::End();
#endif
}