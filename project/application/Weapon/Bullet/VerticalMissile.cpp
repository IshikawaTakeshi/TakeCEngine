#include "VerticalMissile.h"
#include "engine/base/TakeCFrameWork.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/MatrixMath.h"

void VerticalMissile::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	
	Bullet::Initialize(object3dCommon, filePath);

	speed_ = 200.0f; // 初期速度を設定
	isActive_ = true; // 初期状態でアクティブにする
	lifeTime_ = 5.0f; // ライフタイムを設定
	bulletradius_ = 1.5f; // 弾の半径を設定
}

void VerticalMissile::Update() {



}

void VerticalMissile::UpdateImGui() {}

void VerticalMissile::Draw() {
	if (!isActive_) return; // 弾が無効化されている場合は描画しない
	object3d_->Draw();
}

void VerticalMissile::DrawCollider() {

	collider_->DrawCollider();
}

void VerticalMissile::OnCollisionAction(GameCharacter* other) {
	other;
}

void VerticalMissile::Create(const Vector3& weaponPos, const Vector3& targetPos, const float& speed, CharacterType type) {


	transform_.translate = weaponPos;
	characterType_ = type;
	speed_ = speed;
	targetPos_ = targetPos;
	//ターゲットまでの方向を求める
	direction_ =  Vector3Math::Normalize(targetPos_ - transform_.translate);
	isActive_ = true;
}
