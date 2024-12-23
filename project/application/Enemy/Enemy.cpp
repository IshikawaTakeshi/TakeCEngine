#define NOMINMAX
#include "Enemy.h"
#include "Collision/CollisionTypeIdDef.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"

#include "Vector3Math.h"

#include "CameraManager.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "WinApp.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numbers>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Enemy::~Enemy() {
}

void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	Collider::SetTypeID(static_cast<uint32_t>(CollisionTypeIdDef::kEnemy));
	Object3d::Initialize(object3dCommon, filePath);

	transform_.translate = { 0.0f, 1.0f, 20.0f };
	transform_.scale = { 4.0f, 4.0f, 4.0f };

	damageCoolTime_ = 10;
	hp_ = maxHP_;;

	// 赤色に設定
	color_ = { 1.0f, 0.2f, 0.2f, 1.0f };
	model_->GetMesh()->GetMaterial()->SetMaterialDataColor(color_);
}

void Enemy::Update() {
	Attack();
	Move();


	Object3d::Update();
}

void Enemy::Draw() {


	for (auto& bullet : enemyBullet_) {
		bullet->Draw();
	}

	// モデル描画
	Object3d::Draw();
}

void Enemy::Move() {

	velocity_ = { 0, 0, 0 };

#pragma region 3.攻撃処理

	attackInterval_--;

	if (attackInterval_ <= 0) {
		// 攻撃を開始
		attackTime_ = 0;      // 攻撃時間を初期化
		attackInterval_ = 180; // 攻撃間隔をリセット
	}

#pragma endregion
}

void Enemy::UpdateImGui() {
}

void Enemy::OnCollision(Collider* other) {

	// 衝突相手の種別IDを取得
	const uint32_t typeID = other->GetTypeID();

	// 衝突相手がプレイヤーの場合
	if (typeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayer) ||
		typeID == static_cast<uint32_t>(CollisionTypeIdDef::kPlayerBullet)) {
		isAlive_ = false;
	}
}

void Enemy::Attack() {


	const float totalFrames = 480.0f;                                                           // 総フレーム数
	const float angleLimit = std::numbers::pi_v<float> / 4.0f;                                  // 回転角度の制限（±45度）
	const int oscillations = 3;                                                                 // 往復回数
	const float angularSpeed = (2.0f * oscillations * std::numbers::pi_v<float>) / totalFrames; // サイン波の角周波数

	// attackTime_に応じた回転角度の計算（ラジアン）
	float rotationAngle = angleLimit * sin(angularSpeed * attackTime_);
	transform_.rotate.y = rotationAngle;

	Vector3 velocity = { 0.0f, 0.0f, -0.4f };

	Vector3 bulletPos = { transform_.translate.x, transform_.translate.y - 2.5f, transform_.translate.z };

	// 速度ベクトルを自機の向きに合わせて回転
	velocity = MatrixMath::TransformNormal(velocity, worldMatrix_);


	if (attackTime_ <= 480) {

		// 10フレームごとに弾の生成（調整可能）
		if (attackTime_ % 30 == 0) {

			EnemyBullet* newBullet = new EnemyBullet();
			newBullet->Initialize(Object3dCommon::GetInstance(), bulletPos, velocity);
			enemyBullet_.push_back(newBullet);
		}
	}

	// 弾の更新
	for (auto& bullet : enemyBullet_) {
		bullet->StraightRockUpdate();
	}

	attackTime_++;
}

Vector3 Enemy::GetWorldPos() {
	// ワールド行列の平行移動成分を取得(ワールド座標)
	Vector3 worldPos{ worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2] };

	return worldPos;
}
