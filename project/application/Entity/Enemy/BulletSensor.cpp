#include "BulletSensor.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"

void BulletSensor::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<SphereCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetRadius(5.0f); // 半径を設定
	collider_->SetCollisionLayerID(static_cast<uint32_t>(CollisionLayer::Sensor)); // 種別IDを設定
	//水色に設定
	collider_->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f }); // 水色
	//キャラクタータイプ設定
	characterType_ = CharacterType::BULLET_SENSOR;
}

void BulletSensor::Update() {
	// Object3dの更新
	object3d_->Update();
	// コライダーの更新
	collider_->Update(object3d_.get());
}

void BulletSensor::Draw() {
	// Object3dの描画
	object3d_->Draw();
}

void BulletSensor::DrawCollider() {
	// コライダーの描画
	collider_->DrawCollider();
}

void BulletSensor::OnCollisionAction(GameCharacter* other) {

	if(other->GetCharacterType() == CharacterType::PLAYER ||
		other->GetCharacterType() == CharacterType::PLAYER_BULLET || 
		other->GetCharacterType() == CharacterType::PLAYER_MISSILE) {
		
		isActive_ = true; // センサー有効化
		collider_->SetColor({ 1.0f,0.0f,0.0f,1.0f }); // 赤色
	} else {
		
		isActive_ = false; // センサー無効
		return;
	}
}
