#include "BulletSensor.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/base/ImGuiManager.h"

//===================================================================================
//　初期化
//===================================================================================
void BulletSensor::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<SphereCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetRadius(20.0f); // 半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Sensor)); // 種別IDを設定
	//水色に設定
	collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 水色
	//キャラクタータイプ設定
	characterType_ = CharacterType::BULLET_SENSOR;
}

//===================================================================================
//　更新
//===================================================================================
void BulletSensor::Update() {

	// センサーが有効な場合、タイマーを進める
	if(isActive_) {
		activeTimer_ += TakeCFrameWork::GetDeltaTime();
		if (activeTimer_ >= activeDuration_) {
			isActive_ = false;
			activeTimer_ = 0.0f; // タイマーリセット
			collider_->SetColor({ 0.0f,1.0f,1.0f,1.0f }); // 水色に戻す
		}
	}

	// Object3dの更新
	object3d_->Update();
	// コライダーの更新
	collider_->Update(object3d_.get());
}

//===================================================================================
//　ImGuiの更新
//===================================================================================
void BulletSensor::UpdateImGui() {

	ImGui::SeparatorText("bulletSensor");
	//Scale, Rotate, Translate表示
	ImGui::Text("Scale: %.2f, %.2f, %.2f", object3d_->GetScale().x, object3d_->GetScale().y, object3d_->GetScale().z);
	ImGui::Text("Rotate: %.2f, %.2f, %.2f", object3d_->GetRotate().x, object3d_->GetRotate().y, object3d_->GetRotate().z);
	ImGui::Text("Translate: %.2f, %.2f, %.2f", object3d_->GetTranslate().x, object3d_->GetTranslate().y, object3d_->GetTranslate().z);
}

//===================================================================================
//　描画
//===================================================================================
void BulletSensor::Draw() {
	// Object3dの描画
	object3d_->Draw();
}

//===================================================================================
//　コライダーの描画
//===================================================================================
void BulletSensor::DrawCollider() {
	// コライダーの描画
	collider_->DrawCollider();
}

//===================================================================================
//　衝突時の処理
//===================================================================================
void BulletSensor::OnCollisionAction(GameCharacter* other) {

	// 衝突相手がプレイヤー関連の場合、センサーを有効化
	if(other->GetCharacterType() == CharacterType::PLAYER ||
		other->GetCharacterType() == CharacterType::PLAYER_BULLET || 
		other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		
		isActive_ = true; // センサー有効化
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f }); // 赤色
	}
}
